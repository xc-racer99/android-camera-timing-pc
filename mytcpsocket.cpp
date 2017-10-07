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

#include <QFile>
#include <QTcpSocket>

#include "pipeline.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

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
    while(socket->isOpen() && socket->waitForReadyRead(-1) && socket->bytesAvailable()) {
        // Read the first 64 bits - timestamp
        QByteArray timeStampBytes = socket->read(8);
        qint64 timestamp = bytesToLong(timeStampBytes);
        qDebug("reading timestamp %lld", timestamp);

        // Read the next 64 bits - the size of the image
        if(socket->waitForReadyRead()) {
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

                file.close();

                // Run the OCR
                cv::Mat image = cv::imread(file.fileName().toLatin1().constData(), 1);
                int bibNumber = 0;
                if (image.empty()) {
                    qDebug("ERROR:Failed to open image file %s", file.fileName().toLatin1().constData());
                } else {
                    pipeline::Pipeline pipeline;
                    pipeline.setDirectory((directory + "../../tempImages/").toLatin1().constData());
                    std::vector<int> bibNumbers;
                    QString svmModel = "";
                    QFile svmFile(directory + "../../svm.xml");
                    if(svmFile.exists())
                        svmModel = directory + "../../svm.xml";
                    pipeline.processImage(image, svmModel.toLatin1().constData(), /*darkOnLight*/ 1, bibNumbers);
                    if(!bibNumbers.empty()) {
                        if(fromBehind)
                            bibNumber = bibNumbers.back();
                        else
                            bibNumber = bibNumbers.front();
                    }
                }

                emit newImage(file.fileName(), bibNumber);
            }
        }
    }
    emit serverStatus("Disconnected");
    emit finished();
}

void MyTcpSocket::setAtBack(bool atBack) {
    fromBehind = atBack;
}

qint64 MyTcpSocket::bytesToLong(QByteArray b) {
    qint64 result = 0;
    for (int i = 0; i < 8; i++) {
        result <<= 8;
        result |= (b.at(i) & 0xFF);
    }
    return result;
}
