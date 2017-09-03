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
    while (socket->isOpen()) {
        if (socket->bytesAvailable()) {
            // Read the first 64 bits - timestamp
            QString timestamp = socket->read(1);

            // Read the next 64 bits - the size of the image
            QByteArray unprocessedBytes = socket->read(1);
            long bytes = bytesToLong(unprocessedBytes);

            // Actually read the image and save to a file
            QFile file(directory + timestamp + ".jpg");
            file.open(QIODevice::WriteOnly);
            file.write(socket->read(bytes));
            file.flush();
            file.close();
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
