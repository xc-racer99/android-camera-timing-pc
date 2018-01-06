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

MyTcpSocket::MyTcpSocket(QString host, QString dir) {
    hostName = host;
    directory = dir;
}

qint64 MyTcpSocket::readLong() {
    qDebug("Reading long");

    // FIXME have a timouet here too
    while(socket->isOpen() && socket->bytesAvailable() < 8) {
        qDebug("Bytes available %d", socket->bytesAvailable());
        QThread::sleep(1);
    }
    return bytesToLong(socket->read(8));
}

void MyTcpSocket::sendCommand(qint64 cmd) {
    char buf[8];
    longToBytes(cmd, buf);
    socket->write(buf, 8);
    socket->flush();
}

void MyTcpSocket::process() {
    emit serverStatus("Connecting");
    socket = new QTcpSocket(this);
    socket->connectToHost(hostName, 54321);

    if(socket->waitForConnected(5000)) {
        qDebug() << tr("Connected %1").arg(hostName);
        emit serverStatus("Connected");
    } else {
        qDebug() << tr("Failed to connect to %1").arg(hostName);
    }

    // Try sending our initial command
    sendCommand(PC_REQUEST_NEXT);

    while(socket->isOpen() && (socket->bytesAvailable() > 0 || socket->waitForReadyRead(5000))) {
        bool noError = true;

        qDebug("Got here 0");

        // Make sure we're receiving what we want to
        int cmd = readLong();

        // Check if we're receiving data or not
        if(cmd == NO_DATA) {
            qDebug("No data available, trying again");
            sendCommand(PC_ACK);
            QThread::sleep(1);

            // Try sending our next command
            sendCommand(PC_REQUEST_NEXT);

            continue;
        }

        if(cmd != PHONE_IMAGE)
            noError = false;

        // Read image ID, ignored for now
        qint64 imageId = readLong();

        // Read timestamp and make sure it's sane
        qint64 timestamp = readLong();
        qDebug("reading timestamp %lld", timestamp);
        qint64 now = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
        if(llabs(now - timestamp) > 43200000) {
            qDebug("Warning: Off by more than 12hrs.  Flushing...");
            while(socket->bytesAvailable() > 0)
                socket->readAll();
            noError = false;
        }

        // Read file size
        qint64 imageSize = readLong();
        qDebug("file size is %lld", imageSize);

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
            while(socket->bytesAvailable() > 0)
                socket->readAll();
            file.close();
        } else {
            emit newImage(file.fileName());
            file.close();
        }

        // Send ACK
        sendCommand(PC_ACK);

        // Try sending our next command
        sendCommand(PC_REQUEST_NEXT);
    }
    qDebug("Disconnected");
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

void MyTcpSocket::longToBytes(qint64 l, char* result) {
    for (int i = 7; i >= 0; i--) {
        result[i] = (char)(l & 0xFF);
        l >>= 8;
    }
}
