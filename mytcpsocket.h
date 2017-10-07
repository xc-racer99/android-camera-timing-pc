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
    void newImage(QString filename, int bibNumber);
    void serverStatus(QString status);
public slots:
    void process();
    void setAtBack(bool atBack);
private:
    QTcpSocket *socket;
    QString hostName;
    QString directory;

    bool fromBehind;

    qint64 bytesToLong(QByteArray b);
};

#endif // MYTCPSOCKET_H
