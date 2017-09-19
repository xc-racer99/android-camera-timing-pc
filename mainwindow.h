#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QBoxLayout>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QString>

#include "summitemulator.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
private:
    QString directory;
    QBoxLayout *layout;

    SummitEmulator *summit;
    int summitDeviceNumber;
    int nextChannelNum;
signals:

public slots:
private slots:
    void changeSvmModel();
    void getSummitInfo();
    void newTimingPoint();
    void quit();
};

#endif // MAINWINDOW_H
