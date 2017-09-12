#ifndef TIMINGPOINT_H
#define TIMINGPOINT_H

#include <QDialog>
#include <QFile>
#include <QGroupBox>
#include <QHash>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QWidget>

#include "timingcamera.h"

class TimingPoint : public QGroupBox
{
    Q_OBJECT
public:
    explicit TimingPoint(QString directory, QString name, QString ip, QString secondIp, int maxNum, QWidget *parent = 0);
    ~TimingPoint();
private:
    void startBackgroundThread(QString ip, QString name);
    QString roundTime(QTime time, int nth);

    // Common properties
    TimingCamera *mainCamera;
    TimingCamera *secondCamera;
    QFile *csvFile;
    QLabel *timestamp;
    QLineEdit *bibNumEdit;
    QPushButton *nextButton;
    QSlider *imageSlider;
    QString subDirectory;
    QStringList bibsUsed;
    int maxViews;
    int sliderMax;
    QHash<QString, QString> hash;

signals:
    void changeImage(int index);
private slots:
    void saveSettings();
    void incrementSliderMax();
    void doIncrementSliderMax();
    void updateImageInfo(int index);
    // Button handlers
    void submitButtonPushed();
    void plusButtonPushed();
    void minusButtonPushed();
public slots:
};

#endif // TIMINGPOINT_H
