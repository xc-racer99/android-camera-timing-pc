#ifndef IMAGETHREAD_H
#define IMAGETHREAD_H

#include <QThread>
#include <QObject>

class ImageThread : public QThread
{
public:
    explicit ImageThread(QString ip, QString folder);

    // overriding the QThread's run() method
    void run();
private:
    QString ipAddress;
    QString directory;

};

#endif // IMAGETHREAD_H
