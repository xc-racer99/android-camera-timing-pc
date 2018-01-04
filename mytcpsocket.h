#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QObject>
#include <QTcpSocket>

class MyTcpSocket : public QObject
{
    Q_OBJECT
public:
    MyTcpSocket(QString host, QString dir);
    ~MyTcpSocket()
    {
        socket->close();
    }

signals:
    void finished();
    void newImage(QString filename);
    void serverStatus(QString status);
public slots:
    void process();
private:
    QTcpSocket *socket;
    QString hostName;
    QString directory;

    qint64 bytesToLong(QByteArray b);
    void longToBytes(qint64 l, char* result);
    qint64 readLong();
    void sendCommand(qint64 cmd);

    // Errors
    int NO_DATA = 1;
    int NOT_IMPLEMENTED = 2;

    // Commands from phone
    int PHONE_IMAGE = 1001;

    // Commands from PC
    int PC_ACK = 2001;
    int PC_REQUEST_NEXT = 2002;
    int PC_REQUEST_SPECIFIC = 2003;
    int PC_REQUEST_ALL = 2004;
};

#endif // MYTCPSOCKET_H
