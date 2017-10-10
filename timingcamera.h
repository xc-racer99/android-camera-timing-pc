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

    void setAtBack(bool fromBehind);
    void addBlankImage(qint64 time);

    struct Entry {
        QString file;
        int bibNumber;
        qint64 timestamp;

        Entry(QString fileName, int bibNum, qint64 time) : file(fileName), bibNumber(bibNum), timestamp(time) {}
    };

    QGroupBox *statusBox;
    QWidget *imageHolder;
    QList<Entry> entries;
    QLabel *ipAddress;
private:
    QLabel *actualImage;
    QLabel *serverStatus;
    QPushButton *reconnectButton;
    QPushButton *changeSettingsButton;
    QPushButton *minusButton;
    QPushButton *plusButton;
    QScrollArea *scrollArea;
    QString directory;

    bool fromBack;
    float scaleFactor;

    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    void scaleImage(double factor);
    void startBackgroundThread();

signals:
    void settingsChanged(QString newIp);
    void newImage();
public slots:
    void changeImage(int index);
private slots:
    void addNewImage(QString fileName, int bibNumber);
    void changeSettings();
    void reconnectToServer();
    void setConnectionStatus(QString status);
    void zoomIn();
    void zoomOut();
};

#endif // TIMINGCAMERA_H
