#ifndef TIMINGPOINT_H
#define TIMINGPOINT_H

#include <QDialog>
#include <QFile>
#include <QGroupBox>
#include <QHash>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QSlider>
#include <QWidget>

#include "timingcamera.h"

class TimingPoint : public QGroupBox
{
    Q_OBJECT
public:
    struct CameraInfo {
        QString name;
        QString ip;
        bool atBack;

        CameraInfo(QString cameraName, QString ipAddress, bool atBackArg) : name(cameraName), ip(ipAddress), atBack(atBackArg) {}
    };

    explicit TimingPoint(QString directory, QString name, QList<CameraInfo> cameras, int maxNum, int channelNum, QWidget *parent = 0);
    ~TimingPoint();

private:
    void startBackgroundThread(QString ip, QString name);
    QString roundTime(QTime time, int nth);

    // Common properties
    QList<TimingCamera*> timingCameras;
    QFile *csvFile;
    QLabel *timestamp;
    QLineEdit *bibNumEdit;
    QPushButton *nextButton;
    QSlider *imageSlider;
    QString subDirectory;
    int channel;
    QStringList bibsUsed;
    int maxViews;
    QHash<QString, QString> hash;

signals:
    void changeImage(int index);
    void newEntry(int channel, QString bib, QString time);
private slots:
    void saveSettings();
    void incrementSliderMax();
    void updateImageInfo(int index);
    // Button handlers
    void submitButtonPushed();
    void plusButtonPushed();
    void minusButtonPushed();
public slots:
};

#endif // TIMINGPOINT_H
