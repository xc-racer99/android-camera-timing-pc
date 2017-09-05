#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QBoxLayout>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QString>

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
signals:

public slots:
private slots:
    void newTimingPoint();
    void quit();
};

#endif // MAINWINDOW_H
