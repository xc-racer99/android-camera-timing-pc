/**
 * Copyright (C) 2017  Jonathan Bakker
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 **/

#include <QApplication>
#include <QDir>
#include <QDialog>
#include <QDialogButtonBox>
#include <QErrorMessage>
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
    channelNum = 1;

    // Create a file dialog box
    QFileDialog *fileDialog;
    while(directory.isEmpty()) {
        fileDialog = new QFileDialog();
        fileDialog->setFileMode(QFileDialog::Directory);
        if(fileDialog->exec()) {
            directory = fileDialog->selectedFiles().at(0) + QDir::separator();
        }
    }
    delete fileDialog;

    // Create a dialog asking about summit emulation
    QDialog *summitInfo = new QDialog();
    QFormLayout *dialogLayout = new QFormLayout(summitInfo);
    QLabel *serialPortNumLabel = new QLabel(summitInfo);
    serialPortNumLabel->setText(tr("COM Port Number:"));
    QLineEdit *serialPortNum = new QLineEdit(summitInfo);
    dialogLayout->addRow(serialPortNumLabel, serialPortNum);

    QLabel *deviceNumberLabel = new QLabel(summitInfo);
    deviceNumberLabel->setText(tr("Device Number:"));
    QLineEdit *deviceNumber = new QLineEdit(summitInfo);
    dialogLayout->addRow(deviceNumberLabel, deviceNumber);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok,
                               Qt::Horizontal, summitInfo);
    dialogLayout->addRow(&buttonBox);

    summitInfo->setLayout(dialogLayout);

    connect(&buttonBox, SIGNAL(accepted()), summitInfo, SLOT(accept()));

    if(summitInfo->exec() == QDialog::Accepted) {
        // Save things
#ifdef Q_OS_WIN
        QString temp = QString("COM%1").arg(serialPortNum->text());
#else
        QString temp = serialPortNum->text();
#endif
        QSerialPortInfo info(temp);
        summit = new SummitEmulator(info);
        summitDeviceNumber = deviceNumber->text().toInt();
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
    QScrollArea *scrollArea = new QScrollArea(this);
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
            QString ip = in.readLine();
            QString secondIp = in.readLine();
            QString maxViewsString = in.readLine();
            bool ok;
            int maxViews = maxViewsString.toInt(&ok);
            QString channelString = in.readLine();
            int channel = channelString.toInt();
            TimingPoint *tPoint;
            if(secondIp == NULL)
                tPoint = new TimingPoint(directory, subDirs.at(i).baseName(), ip, "", 1, channelNum++, this);
            else if(maxViewsString == NULL || !ok)
                tPoint = new TimingPoint(directory, subDirs.at(i).baseName(), ip, secondIp, 1, channelNum++, this);
            else if(channelString == NULL)
                tPoint = new TimingPoint(directory, subDirs.at(i).baseName(), ip, secondIp, maxViews, channelNum++, this);
            else {
                tPoint = new TimingPoint(directory, subDirs.at(i).baseName(), ip, secondIp, maxViews, channel, this);
                channelNum = qMax(channelNum, channel);
            }
            layout->addWidget(tPoint);
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

    // Secondary IP
    QLineEdit *secondIp = new QLineEdit(&dialog);
    QLabel secondIpLabel(tr("Second IP:"));
    formLayout.addRow(&secondIpLabel, secondIp);

    // # of times we can see the same bib
    QLineEdit *numViews = new QLineEdit(&dialog);
    QLabel numViewsLabel(tr("Max times through this point:"));
    formLayout.addRow(&numViewsLabel, numViews);

    // Summit channel number
    QLineEdit *channelNumber = new QLineEdit(&dialog);
    QLabel channelNumberLabel(tr("Summit Channel Number:"));
    formLayout.addRow(&channelNumberLabel, channelNumber);

    // Buttons
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    formLayout.addRow(&buttonBox);

    // Make connections
    connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    if (dialog.exec() == QDialog::Accepted) {
        if(pointName->text().isEmpty() || mainIp->text().isEmpty() || numViews->text().isEmpty()) {
            QMessageBox msgBox;
            msgBox.setText(tr("You must specify a point name, an IP, and the max views"));
            msgBox.exec();
            return;
        }

        TimingPoint *tPoint = new TimingPoint(directory,pointName->text(),
                                              mainIp->text(), secondIp->text(),
                                              numViews->text().toInt(), channelNumber->text().toInt(), this);
        connect(tPoint, SIGNAL(newEntry(int,QString,QString)), this, SLOT(newSummitEntry(int,QString,QString)));
        layout->addWidget(tPoint);
    }
}

void MainWindow::newSummitEntry(int channel, QString bib, QString time) {
    summit->sendData(summitDeviceNumber, channel, bib, time);
}
