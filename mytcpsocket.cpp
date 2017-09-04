#include <QFile>
#include <QTcpSocket>
#include "mytcpsocket.h"

MyTcpSocket::MyTcpSocket(QObject *parent) : QObject(parent)
{

}

void MyTcpSocket::doConnect(QString host) {
    socket = new QTcpSocket(this);
    socket->connectToHost(host, 54321);

    if(socket->waitForConnected(50000)) {
        qDebug() << tr("Connected %1").arg(host);
    } else {
        qDebug() << tr("Failed to connect to %1").arg(host);
    }
}

void MyTcpSocket::saveImages(QString directory) {
    while(socket->isOpen() && socket->waitForReadyRead(-1) && socket->bytesAvailable()) {
        // Read the first 64 bits - timestamp
        QByteArray timeStampBytes = socket->read(8);
        long timestamp = bytesToLong(timeStampBytes);
        qDebug("reading timestamp %ld", timestamp);

        // Read the next 64 bits - the size of the image
        if(socket->waitForReadyRead()) {
            QByteArray imageSizeBytes = socket->read(8);
            long imageSize = bytesToLong(imageSizeBytes);
            qDebug("file size is %ld", imageSize);

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
                emit newImage(file.fileName());
            }
        }
    }
}

long MyTcpSocket::bytesToLong(QByteArray b) {
    long result = 0;
    for (int i = 0; i < 8; i++) {
        result <<= 8;
        result |= (b.at(i) & 0xFF);
    }
    return result;
}
