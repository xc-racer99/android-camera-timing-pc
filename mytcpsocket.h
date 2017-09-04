#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QObject>
#include <QTcpSocket>

class MyTcpSocket : public QObject
{
    Q_OBJECT
public:
    MyTcpSocket(QString host, QString dir);
signals:
    void finished();
    void newImage(QString filename);
    void connected();
public slots:
    void process();
private:
    QTcpSocket *socket;
    QString hostName;
    QString directory;

    long bytesToLong(QByteArray b);
};

#endif // MYTCPSOCKET_H
