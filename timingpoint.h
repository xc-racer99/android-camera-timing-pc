#ifndef TIMINGPOINT_H
#define TIMINGPOINT_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QDialog>
#include <QLineEdit>
#include <QFile>
#include <QGroupBox>

#include "timingcamera.h"

class TimingPoint : public QGroupBox
{
    Q_OBJECT
public:
    explicit TimingPoint(QString directory, QString name, QString ip, QString secondIp, QWidget *parent = 0);
private:
    void startBackgroundThread(QString ip, QString name);

    // Common properties
    TimingCamera *mainCamera;
    TimingCamera *secondCamera;
    QFile *csvFile;
    QLabel *timestamp;
    QLineEdit *bibNumEdit;
    QPushButton *nextButton;
    QSlider *imageSlider;
    QString subDirectory;

signals:
    void changeImage(int index);
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
