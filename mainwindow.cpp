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
#include <QComboBox>
#include <QDir>
#include <QDialog>
#include <QDialogButtonBox>
#include <QErrorMessage>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QList>
#include <QMessageBox>
#include <QSettings>
#include <QScrollArea>
#include <QTextStream>

#include "mainwindow.h"
#include "timingpoint.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    nextChannelNum = 1;

    QSettings settings;

    // Create a file dialog box
    QFileDialog *fileDialog;
    while(directory.isEmpty()) {
        fileDialog = new QFileDialog();
        fileDialog->setDirectory(settings.value("last_dir_used").toString());
        fileDialog->setFileMode(QFileDialog::Directory);
        if(fileDialog->exec()) {
            directory = fileDialog->selectedFiles().at(0) + QDir::separator();
        }
    }
    delete fileDialog;

    // Save the folder
    settings.setValue("last_dir_used", directory);

    // Create the summit
    summit = new SummitEmulator;

    // Set the menu bars
    // Initialize variables
    QMenuBar *menubar = new QMenuBar(this);
    QMenu *menuFile = new QMenu(menubar);
    QAction *actionNewTimingPoint = new QAction(this);
    QAction *actionQuit = new QAction(this);
    QMenu *menuSummit = new QMenu(menubar);
    QAction *actionSummitSettings = new QAction(this);
    QMenu *ocrSettings = new QMenu(this);
    QAction *svmModel = new QAction(this);

    // Set the text
    actionNewTimingPoint->setText("New Timing Point");
    actionQuit->setText("Quit");
    menuFile->setTitle("File");
    menuSummit->setTitle(tr("Summit"));
    actionSummitSettings->setText(tr("Settings"));
    ocrSettings->setTitle(tr("OCR"));
    svmModel->setText(tr("Choose SVM Model..."));

    // Add the menu items to the file menu and it to the menu
    menubar->addAction(menuFile->menuAction());
    menubar->addAction(menuSummit->menuAction());
    menubar->addAction(ocrSettings->menuAction());
    menuFile->addAction(actionNewTimingPoint);
    menuFile->addAction(actionQuit);
    menuSummit->addAction(actionSummitSettings);
    ocrSettings->addAction(svmModel);

    // Set the menu bar
    this->setMenuBar(menubar);

    // Make connections
    connect(actionNewTimingPoint, SIGNAL(triggered(bool)), this, SLOT(newTimingPoint()));
    connect(actionQuit, SIGNAL(triggered(bool)), this, SLOT(quit()));
    connect(actionSummitSettings, SIGNAL(triggered(bool)), this, SLOT(getSummitInfo()));
    connect(svmModel, SIGNAL(triggered(bool)), this, SLOT(changeSvmModel()));

    // Create the layout
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    QWidget *container = new QWidget(scrollArea);
    scrollArea->setWidget(container);
    layout = new QVBoxLayout(container);
    setCentralWidget(scrollArea);

    QDir dir(directory);

    // Create a folder for temporary images
    if(!dir.exists("tempImages"))
        dir.mkpath("tempImages");

    QFile saveQFile(directory + "/save.xml");
    if(saveQFile.exists()) {
        if(!saveQFile.open(QIODevice::ReadOnly)) {
            // Show error about opening save file
            QMessageBox msgBox;
            msgBox.setText(tr("Failed to open the save file %1.  Please check that you have read permissions").arg(saveQFile.fileName()));
            msgBox.exec();
            return;
        }

        QDomDocument saveFile;
        if(!saveFile.setContent(&saveQFile)) {
            // Show error reading save file
            QMessageBox msgBox;
            msgBox.setText(tr("Failed to read the save file %1.  Please make sure it is a valid XML file.").arg(saveQFile.fileName()));
            msgBox.exec();
            saveQFile.close();
            return;
        }
        saveQFile.close();

        QDomElement docElem = saveFile.documentElement();
        int summitDeviceNumber = docElem.attribute("devicenumber", "0").toInt();
        summit->setDeviceNumber(summitDeviceNumber);
        QDomElement n = docElem.firstChildElement("TimingPoint");
        while(!n.isNull()) {
            QString name = n.attribute("name");
            QString maxViews = n.attribute("maxviews", "1");
            QString summitChannel = n.attribute("channel", "1");
            QList<TimingPoint::CameraInfo> info;
            QDomElement child = n.firstChildElement("TimingCamera");
            while(!child.isNull()) {
                QString cameraName = child.firstChildElement("Name").text();
                QString cameraIp = child.firstChildElement("IP").text();
                bool atBack = child.firstChildElement("AtBack").text() == "true";
                qint64 timeOffset = child.firstChildElement("TimeOffset").text().toLongLong();
                info.append(TimingPoint::CameraInfo(cameraName, cameraIp, timeOffset, atBack));
                child = child.nextSiblingElement("TimingCamera");
            }
            TimingPoint *tPoint = new TimingPoint(directory, name, info, maxViews.toInt(), summitChannel.toInt(), this);
            connect(tPoint, SIGNAL(newEntry(int,QString,QString)), summit, SLOT(sendData(int,QString,QString)));
            connect(tPoint, SIGNAL(settingsChanged()), this, SLOT(saveSettings()));
            tPoints.append(tPoint);
            layout->addWidget(tPoint);
            n = n.nextSiblingElement("TimingPoint");
        }
        saveSettings();
    }

    // Create a dialog asking about summit emulation
    getSummitInfo();
}

void MainWindow::quit() {
    QApplication::quit();
}

void MainWindow::saveSettings() {
    // Save an XML file
    QFile file(directory + "save.xml");
    if(!file.open(QIODevice::WriteOnly)) {
        // Show error about opening save file
        QMessageBox msgBox;
        msgBox.setText(tr("Failed to write the save file %1.  Please check that you have write permissions").arg(file.fileName()));
        msgBox.exec();
        return;
    }
    QDomDocument saveFile;
    QDomElement root = saveFile.createElement("CameraTiming");
    root.setAttribute("devicenumber", summit->getDeviceNumber());
    saveFile.appendChild(root);
    for(int i = 0; i < tPoints.length(); i++) {
        TimingPoint *tPoint = tPoints.at(i);
        QDomElement point = saveFile.createElement("TimingPoint");
        point.setAttribute("name", tPoint->getTitle());
        point.setAttribute("maxviews", tPoint->getMaxViews());
        point.setAttribute("channel", tPoint->getChannel());

        QList<TimingPoint::CameraInfo> cameraInfo = tPoint->getCameraInfo();
        for(int j = 0; j < cameraInfo.length(); j++) {
            QDomElement camera = saveFile.createElement("TimingCamera");
            point.appendChild(camera);
            QDomElement name = saveFile.createElement("Name");
            camera.appendChild(name);
            QDomText nameText = saveFile.createTextNode(cameraInfo.at(j).name);
            name.appendChild(nameText);
            QDomElement ip = saveFile.createElement("IP");
            camera.appendChild(ip);
            QDomText ipText = saveFile.createTextNode(cameraInfo.at(j).ip);
            ip.appendChild(ipText);
            QDomElement atBack = saveFile.createElement("AtBack");
            camera.appendChild(atBack);
            bool fromBehind = cameraInfo.at(j).atBack;
            if(fromBehind) {
                QDomText atBackText = saveFile.createTextNode("true");
                atBack.appendChild(atBackText);
            } else {
                QDomText atBackText = saveFile.createTextNode("false");
                atBack.appendChild(atBackText);
            }
            QDomElement timeOffset = saveFile.createElement("TimeOffset");
            camera.appendChild(timeOffset);
            QDomText timeOffsetText = saveFile.createTextNode(QString("%1").arg(cameraInfo.at(j).offset));
            timeOffset.appendChild(timeOffsetText);
        }
        root.appendChild(point);
    }

    QTextStream out(&file);
    saveFile.save(out, 4);
    file.close();
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
    numViews->setText("1");
    formLayout.addRow(&numViewsLabel, numViews);

    // Summit channel number
    QLineEdit *channelNumber = new QLineEdit(&dialog);
    QLabel channelNumberLabel(tr("Summit Channel Number:"));
    channelNumber->setText(QString("%1").arg(nextChannelNum));
    formLayout.addRow(&channelNumberLabel, channelNumber);

    // Buttons
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    formLayout.addRow(&buttonBox);

    // Make connections
    connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    if (dialog.exec() == QDialog::Accepted) {
        if(pointName->text().isEmpty() || numViews->text().isEmpty()) {
            QMessageBox msgBox;
            msgBox.setText(tr("You must specify a point name and the max views"));
            msgBox.exec();
            return;
        }

        QList<TimingPoint::CameraInfo> cameraInfo;
        cameraInfo.append(TimingPoint::CameraInfo("Main", mainIp->text(), 0, false));
        cameraInfo.append(TimingPoint::CameraInfo("Secondary", secondIp->text(), 0, false));
        TimingPoint *tPoint = new TimingPoint(directory,pointName->text(),
                                              cameraInfo,
                                              numViews->text().toInt(), channelNumber->text().toInt(), this);
        connect(tPoint, SIGNAL(newEntry(int,QString,QString)), summit, SLOT(sendData(int,QString,QString)));
        connect(tPoint, SIGNAL(settingsChanged()), this, SLOT(saveSettings()));
        tPoints.append(tPoint);
        layout->addWidget(tPoint);
        saveSettings();
    }
}

void MainWindow::getSummitInfo() {
    QDialog *summitInfo = new QDialog();
    QFormLayout *dialogLayout = new QFormLayout(summitInfo);

    QLabel *serialPortNumLabel = new QLabel(summitInfo);
    serialPortNumLabel->setText(tr("Serial Port:"));
    QList<QSerialPortInfo> serialPorts = QSerialPortInfo::availablePorts();
    QComboBox *comboBox = new QComboBox(summitInfo);
    for(int i = 0; i < serialPorts.length(); i++) {
        comboBox->addItem(serialPorts.at(i).portName());
    }
    dialogLayout->addRow(serialPortNumLabel, comboBox);

    QLabel *deviceNumberLabel = new QLabel(summitInfo);
    deviceNumberLabel->setText(tr("Summit Device Number:"));
    QLineEdit *deviceNumber = new QLineEdit(summitInfo);
    dialogLayout->addRow(deviceNumberLabel, deviceNumber);

    int currentNum = summit->getDeviceNumber();
    if(currentNum != 0)
        deviceNumber->setText(QString("%1").arg(currentNum));

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok,
                               Qt::Horizontal, summitInfo);
    dialogLayout->addRow(&buttonBox);

    summitInfo->setLayout(dialogLayout);

    connect(&buttonBox, SIGNAL(accepted()), summitInfo, SLOT(accept()));

    if(summitInfo->exec() == QDialog::Accepted) {
        if(serialPorts.length() > 0)
            summit->setPort(serialPorts.at(comboBox->currentIndex()));
        bool ok;
        int deviceNum = deviceNumber->text().toInt(&ok);
        if(ok)
            summit->setDeviceNumber(deviceNum);
        saveSettings();
    }
}

void MainWindow::changeSvmModel() {
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open SVM Model"), QString(), tr("SVM Files (*.xml)"));
    if(!fileName.isEmpty()) {
        // Remove the existing xml before copying the new one
        QFile svmFile(directory + "svm.xml");
        if(svmFile.exists())
            svmFile.remove();
        QFile newSvmFile(fileName);
        newSvmFile.copy(directory + "svm.xml");
    }
}
