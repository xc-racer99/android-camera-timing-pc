#ifndef TIMINGCAMERA_H
#define TIMINGCAMERA_H

#include <QGroupBox>
#include <QLabel>
#include <QObject>
#include <QPushButton>
#include <QWidget>

class TimingCamera : public QObject
{
    Q_OBJECT
public:
    explicit TimingCamera(QString dir, QString ip, QObject *parent = 0);

    QGroupBox *statusBox;
    QLabel *imageHolder;
    QStringList imagePaths;
private:
    QLabel *ipAddress;
    QLabel *serverStatus;
    QPushButton *reconnectButton;
    QPushButton *changeIpButton;
    QString directory;

    void startBackgroundThread();
signals:
    void ipAddressChanged(QString newIp);
    void newImage();
public slots:
    void changeImage(int index);
private slots:
    void addNewImage(QString fileName);
    void changeIpDialog();
    void reconnectToServer();
    void setConnectionStatus(QString status);
};

#endif // TIMINGCAMERA_H
