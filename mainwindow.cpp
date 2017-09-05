#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>

#include "mainwindow.h"
#include "timingpoint.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // Create a file dialog box
    while(directory.isEmpty()) {
        QFileDialog *fileDialog = new QFileDialog();
        fileDialog->setFileMode(QFileDialog::Directory);
        if(fileDialog->exec()) {
            directory = fileDialog->selectedFiles().at(0) + QDir::separator();
        }
    }

    // Set the menu bars
    // Initialize variables
    menubar = new QMenuBar(this);
    menuFile = new QMenu(menubar);
    actionNewTimingPoint = new QAction(this);
    actionQuit = new QAction(this);

    // Set the text
    actionNewTimingPoint->setText("New Timing Point");
    actionQuit->setText("Quit");
    menuFile->setTitle("File");

    // Add the menu items to the file menu and it to the menu
    menubar->addAction(menuFile->menuAction());
    menuFile->addAction(actionNewTimingPoint);
    menuFile->addAction(actionQuit);

    // Set the menu bar
    this->setMenuBar(menubar);

    // Make connections
    connect(actionNewTimingPoint, SIGNAL(triggered(bool)), this, SLOT(newTimingPoint()));
    connect(actionQuit, SIGNAL(triggered(bool)), this, SLOT(quit()));

    // Create the layout
    layout = new QVBoxLayout;
    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);
}

void MainWindow::quit() {
    QApplication::quit();
}

void MainWindow::newTimingPoint() {
        TimingPoint *tPoint = new TimingPoint(directory);
        layout->addWidget(tPoint);
}
