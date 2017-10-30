#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QBoxLayout>
#include <QDomDocument>
#include <QList>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QString>

#include "summitemulator.h"
#include "timingpoint.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
private:
    QString directory;
    QBoxLayout *layout;

    SummitEmulator *summit;
    int nextChannelNum;
    QList<TimingPoint*> tPoints;
signals:
public slots:
private slots:
    void applyParamsElsewhere(DetectText::TextDetectionParams params);
    void changeSvmModel();
    void getSummitInfo();
    void newTimingPoint();
    void quit();
    void saveSettings();
};

#endif // MAINWINDOW_H
