#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QObject>
#include <QTcpSocket>

class MyTcpSocket : public QObject
{
    Q_OBJECT
public:
    explicit MyTcpSocket(QObject *parent = 0);

    void doConnect(QString host);
    void saveImages(QString directory);
    long bytesToLong(QByteArray b);
signals:

public slots:
private:
    QTcpSocket *socket;
};

#endif // MYTCPSOCKET_H
