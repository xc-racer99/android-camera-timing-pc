#include <QDir>
#include "imagethread.h"
#include "mytcpsocket.h"

ImageThread::ImageThread(QString ip, QString folder)
{
    ipAddress = ip;
    directory = folder;

    // Create the folder if it doesn't exist
    QDir dir(directory);
    if(!dir.exists(directory)) {
        dir.mkpath(directory);
    }
}

void ImageThread::run() {
    // Start the connection
    MyTcpSocket socket;
    socket.doConnect(ipAddress);
    socket.saveImages(directory);

    // Daisy-chain the newImage signals
    connect(&socket, SIGNAL(newImage(QString)), this, SIGNAL(newImage(QString)));
    exit();
}

