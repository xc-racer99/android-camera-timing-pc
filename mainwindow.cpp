#include <QApplication>
#include <QDir>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QMessageBox>
#include <QScrollArea>
#include <QTextStream>

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
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    QWidget *container = new QWidget(scrollArea);
    scrollArea->setWidget(container);
    layout = new QVBoxLayout(container);
    setCentralWidget(scrollArea);

    // Check and see if we're opening a folder that's already been in use
    QDir dir(directory);
    QStringList filter("*");
    QFileInfoList subDirs = dir.entryInfoList(filter, QDir::Dirs);
    // Loop through the subdirectories, adding a new timing point for each one that has a .settings file
    for(int i = 0; i < subDirs.length(); i++) {
        QFile settingsFile(subDirs.at(i).absoluteFilePath() + "/.settings");
        if(!settingsFile.exists())
            continue;
        if(settingsFile.open(QFile::ReadOnly)) {
            QTextStream in(&settingsFile);
            newTimingPoint(subDirs.at(i).baseName(), in.readLine());
        }
        settingsFile.close();
    }
}

void MainWindow::quit() {
    QApplication::quit();
}

void MainWindow::newTimingPoint() {
    // Get connection info
    QDialog dialog(this);
    QFormLayout formLayout(&dialog);

    // Point name
    QLineEdit *pointName = new QLineEdit(&dialog);
    QLabel pointNameLabel(tr("Point Name:"));
    formLayout.addRow(&pointNameLabel, pointName);

    // IP Address
    QLineEdit *mainIp = new QLineEdit(&dialog);
    QLabel mainIpLabel(tr("Main IP:"));
    formLayout.addRow(&mainIpLabel, mainIp);

    // Buttons
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    formLayout.addRow(&buttonBox);

    // Make connections
    connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    if (dialog.exec() == QDialog::Accepted) {
        if(pointName->text().isEmpty() || mainIp->text().isEmpty()) {
            QMessageBox msgBox;
            msgBox.setText(tr("You must specify both a point name and an IP"));
            msgBox.exec();
            return;
        }

        TimingPoint *tPoint = new TimingPoint(directory, pointName->text(), mainIp->text());
        layout->addWidget(tPoint);
    }
}

void MainWindow::newTimingPoint(QString name, QString ip) {
    TimingPoint *tPoint = new TimingPoint(directory, name, ip);
    layout->addWidget(tPoint);
}
