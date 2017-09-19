#ifndef TIMINGCAMERA_H
#define TIMINGCAMERA_H

#include <QGroupBox>
#include <QLabel>
#include <QObject>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QWidget>

class TimingCamera : public QObject
{
    Q_OBJECT
public:
    explicit TimingCamera(QString dir, QString ip, QObject *parent = 0);
    ~TimingCamera();

    struct Entry {
        QString file;
        int bibNumber;

        Entry(QString fileName, int bibNum) : file(fileName), bibNumber(bibNum) {}
    };

    QGroupBox *statusBox;
    QWidget *imageHolder;
    QList<Entry> entries;
    QLabel *ipAddress;
private:
    QLabel *actualImage;
    QLabel *serverStatus;
    QPushButton *reconnectButton;
    QPushButton *changeIpButton;
    QPushButton *minusButton;
    QPushButton *plusButton;
    QScrollArea *scrollArea;
    QString directory;

    float scaleFactor;

    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    void scaleImage(double factor);
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
    void zoomIn();
    void zoomOut();
};

#endif // TIMINGCAMERA_H
