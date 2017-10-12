/**
 * Copyright (C) 2017  Jonathan Bakker
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 **/

#include <QDateTime>
#include <QFile>
#include <QTcpSocket>
#include <QThread>

#include "mytcpsocket.h"

MyTcpSocket::MyTcpSocket(QString host, QString dir)
{
    hostName = host;
    directory = dir;
}

void MyTcpSocket::process() {
    emit serverStatus("Connecting");
    socket = new QTcpSocket(this);
    socket->connectToHost(hostName, 54321);

    if(socket->waitForConnected(50000)) {
        qDebug() << tr("Connected %1").arg(hostName);
        emit serverStatus("Connected");
    } else {
        qDebug() << tr("Failed to connect to %1").arg(hostName);
    }
    while(socket->isOpen() && socket->waitForReadyRead(-1)) {
        bool noError = true;

        // Read the first 64 bits - timestamp
        while(socket->isOpen() && socket->waitForReadyRead(5000) && socket->bytesAvailable() < 8)
            true;
        QByteArray timeStampBytes = socket->read(8);
        qint64 timestamp = bytesToLong(timeStampBytes);
        qDebug("reading timestamp %lld", timestamp);
        qint64 now = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
        if(abs(now - timestamp) > 43200000) {
            qDebug("Warning: Off by more than 12hrs.  Flushing...");
            socket->readAll();
            noError = false;
        }

        // Read the next 64 bits - the size of the image
        if(socket->isOpen() && socket->waitForReadyRead(5000) && noError) {
            while(socket->isOpen() && socket->waitForReadyRead(-1) && socket->bytesAvailable() < 8)
                true;
            QByteArray imageSizeBytes = socket->read(8);
            qint64 imageSize = bytesToLong(imageSizeBytes);
            qDebug("file size is %lld", imageSize);

            if(socket->waitForReadyRead(-1)) {
                // Actually read the image and save to a file
                QFile file(directory + QString::number(timestamp) + ".jpg");

                qDebug("Saving to file %s", file.fileName().toLatin1().constData());
                file.open(QIODevice::WriteOnly);

                qint64 bytesLeft = imageSize;
                while(bytesLeft > 0 && (socket->bytesAvailable() || socket->waitForReadyRead(5000))) {
                    bytesLeft -= file.write(socket->read(bytesLeft));
                }

                if(file.size() != imageSize) {
                    qDebug("Warning: Sizes do not match. Flushing...");
                    socket->readAll();
                    file.close();
                } else {
                    emit newImage(file.fileName());
                    file.close();
                }
            }
        }
    }
    emit serverStatus("Disconnected");
    emit finished();
}

qint64 MyTcpSocket::bytesToLong(QByteArray b) {
    qint64 result = 0;
    for (int i = 0; i < 8; i++) {
        result <<= 8;
        result |= (b.at(i) & 0xFF);
    }
    return result;
}
