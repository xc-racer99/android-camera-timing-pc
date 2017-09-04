#include "imagethread.h"
#include "mytcpsocket.h"

ImageThread::ImageThread(QString ip, QString folder)
{
    ipAddress = ip;
    directory = folder;
}

void ImageThread::run() {
    // Start the connection
    MyTcpSocket socket;
    socket.doConnect(ipAddress);
    socket.saveImages(directory);
}

