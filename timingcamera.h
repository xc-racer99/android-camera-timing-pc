#ifndef TIMINGCAMERA_H
#define TIMINGCAMERA_H

#include <QGroupBox>
#include <QLabel>
#include <QObject>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QWidget>

#include "ocrpipeline.h"

class TimingCamera : public QObject
{
    Q_OBJECT
public:
    explicit TimingCamera(QString dir, QString ip, QObject *parent = 0);
    ~TimingCamera();

    bool getAtBack(void);
    QString getIpAddress(void);
    QString getName(void);
    DetectText::TextDetectionParams getParams();
    qint64 getTimestampOffset(void);
#ifndef NO_OCR
    void setAtBack(bool fromBehind);
    void setParams(DetectText::TextDetectionParams params);
#endif
    void setTimestampOffset(qint64 offset);
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
    qint64 timeOffset;

    bool fromBack;
    float scaleFactor;

#ifndef NO_OCR
    OcrPipeline *pipeline;
#endif

    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    void scaleImage(double factor);
    void startBackgroundThread();

signals:
    void applyParamsElsewhere(DetectText::TextDetectionParams params);
    void settingsChanged();
    void newImage();
public slots:
    void changeImage(int index);
    void checkEntries(int numEntries, qint64 timestamp);
private slots:
    void addNewImage(QString fileName, int bibNumber);
    void addNewImageFromSocket(QString fileName);
    void changeSettings();
    void reconnectToServer();
    void setConnectionStatus(QString status);
    void zoomIn();
    void zoomOut();
};

#endif // TIMINGCAMERA_H
