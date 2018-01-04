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

#ifdef NO_OCR
#include "ocrpipeline.h"
#else
#include "3rd-Party/bibnumber/textdetection.h"
#endif

#include "timingcamera.h"

class TimingPoint : public QGroupBox
{
    Q_OBJECT
public:
    struct CameraInfo {
        QString name;
        QString ip;
        qint64 offset;
        bool atBack;
        DetectText::TextDetectionParams params;

        CameraInfo(QString cameraName,
                   QString ipAddress,
                   qint64 timeOffset,
                   bool atBackArg, DetectText::TextDetectionParams parameters) :
            name(cameraName), ip(ipAddress), offset(timeOffset), atBack(atBackArg), params(parameters){}
        CameraInfo(QString cameraName,
                   QString ipAddress,
                   qint64 timeOffset,
                   bool atBackArg) :
            name(cameraName), ip(ipAddress), offset(timeOffset), atBack(atBackArg) {
            params = { true, /* darkOnLight */
                       15, /* maxStrokeLength */
                       11, /* minCharacterHeight */
                       100, /* maxImgWidthToTextRatio */
                       45, /* maxAngle */
                       10, /* topBorder: discard top 10% */
                       5,  /* bottomBorder: discard bottom 5% */
                       3, /* min chain len */
                       0, /* verify with SVM model up to this chain len */
                       0, /* height needs to be this large to verify with model */
                       false /* use new chaining code */
            };
        }
    };

    explicit TimingPoint(QString directory, QString name, QList<CameraInfo> cameras, int maxNum, int channelNum, QWidget *parent = 0);
    ~TimingPoint();

    QList<CameraInfo> getCameraInfo();
    QString getTitle();
    int getChannel();
    int getMaxViews();
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
    void applyParamsElsewhere(DetectText::TextDetectionParams params);
    void changeImage(int index);
    void checkEntries(int maxEntries, qint64 largestTimestamp);
    void newEntry(int channel, QString bib, QString time);
    void settingsChanged();
private slots:
    void changePointSettings();
    void incrementSliderMax();
    void updateImageInfo(int index);
    // Button handlers
    void submitButtonPushed();
    void plusButtonPushed();
    void minusButtonPushed();
public slots:
    void applyParams(DetectText::TextDetectionParams params);
};

#endif // TIMINGPOINT_H
