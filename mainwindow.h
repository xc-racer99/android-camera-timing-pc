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

    // Menu bars and items
    QMenuBar *menubar;
    QMenu *menuFile;
    QAction *actionNewTimingPoint;
    QAction *actionQuit;

    SummitEmulator *summit;
    int summitDeviceNumber;
    int channelNum;
signals:

public slots:
private slots:
    void newSummitEntry(int channel, QString bib, QString time);
    void newTimingPoint();
    void quit();
};

#endif // MAINWINDOW_H
