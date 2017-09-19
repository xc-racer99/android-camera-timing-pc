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

#include <QDir>
#include <QGridLayout>
#include <QImageReader>
#include <QInputDialog>
#include <QScrollArea>
#include <QSlider>
#include <QThread>

#include "mytcpsocket.h"
#include "timingcamera.h"

TimingCamera::TimingCamera(QString dir, QString ip, QObject *parent) : QObject(parent)
{
    directory = dir;

    scaleFactor = 1.0;

    QDir temp(directory);
    if(!temp.exists())
        temp.mkpath(dir);

    ipAddress = new QLabel();
    ipAddress->setText(ip);

    // Create a group box containing the camera status
    statusBox = new QGroupBox();
    statusBox->setTitle(temp.dirName() + tr(" Status"));
    QGridLayout *statusLayout = new QGridLayout(statusBox);

    // Create our info labels
    QLabel *ipAddressLabel = new QLabel(statusBox);
    ipAddressLabel->setText(tr("IP:"));
    QLabel *serverStatusLabel = new QLabel(statusBox);
    serverStatusLabel->setText(tr("Server Status:"));

    // Initialize the variable labels
    ipAddress = new QLabel(statusBox);
    ipAddress->setText(ip);
    serverStatus = new QLabel(statusBox);

    // Initialize push buttons
    reconnectButton = new QPushButton(statusBox);
    reconnectButton->setText(tr("Reconnect"));
    changeIpButton = new QPushButton(statusBox);
    changeIpButton->setText(tr("Change IP"));

    // Setup a group box which holds everything
    imageHolder = new QWidget();
    QGridLayout *layout = new QGridLayout(imageHolder);
    minusButton = new QPushButton(imageHolder);
    minusButton->setIcon(QIcon(":/images/images/minus-icon.png"));
    minusButton->setIconSize(QSize(15, 15));
    minusButton->setFixedSize(QSize(25, 25));
    minusButton->setEnabled(false);
    plusButton = new QPushButton(imageHolder);
    plusButton->setIcon(QIcon(":/images/images/plus-icon.png"));
    plusButton->setIconSize(QSize(15, 15));
    plusButton->setFixedSize(QSize(25, 25));

    actualImage = new QLabel(imageHolder);
    actualImage->setBackgroundRole(QPalette::Base);
    actualImage->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    actualImage->setScaledContents(true);

    scrollArea = new QScrollArea(imageHolder);
    scrollArea->setWidget(actualImage);
    scrollArea->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    scrollArea->setFixedWidth(512);
    scrollArea->setBackgroundRole(QPalette::Base);

    layout->addWidget(scrollArea, 0, 0, 1, 3);
    layout->addWidget(minusButton, 1, 0, 1, 1);
    layout->addWidget(plusButton, 1, 2, 1, 1);

    imageHolder->setLayout(layout);

    // Choose our blank image
    entries.append(Entry(":/images/images/No_image.png", 0));

    statusLayout->addWidget(ipAddressLabel, 0, 0, 1, 1);
    statusLayout->addWidget(ipAddress, 0, 1, 1, 1);
    statusLayout->addWidget(serverStatusLabel, 1, 0, 1, 1);
    statusLayout->addWidget(serverStatus, 1, 1, 1, 1);
    statusLayout->addWidget(reconnectButton, 2, 0, 1, 1);
    statusLayout->addWidget(changeIpButton, 2, 1, 1, 1);
    statusBox->setLayout(statusLayout);

    // Button connections
    connect(reconnectButton, SIGNAL(clicked(bool)), this, SLOT(reconnectToServer()));
    connect(changeIpButton, SIGNAL(clicked(bool)), this, SLOT(changeIpDialog()));
    connect(minusButton, SIGNAL(clicked(bool)), this, SLOT(zoomOut()));
    connect(plusButton, SIGNAL(clicked(bool)), this, SLOT(zoomIn()));

    // Create a list of images already present
    QStringList filter("*.jpg");
    QFileInfoList initialFileInfo = temp.entryInfoList(filter, QDir::Files, QDir::Name);
    for(int i = 0; i < initialFileInfo.length(); i++)
        entries.append(Entry(initialFileInfo.at(i).absoluteFilePath(), 0));

    // Start the image saving thread
    startBackgroundThread();
}

TimingCamera::~TimingCamera() {
    delete ipAddress;
    delete statusBox;
    delete imageHolder;
}

void TimingCamera::reconnectToServer() {
    startBackgroundThread();
}

void TimingCamera::changeIpDialog() {
    bool ok;
    QString newIp = QInputDialog::getText(statusBox, tr("Enter new IP"), tr("IP:"), QLineEdit::Normal, ipAddress->text(), &ok);

    // Update the ip address shown
    if(ok && !newIp.isEmpty()) {
        ipAddress->setText(newIp);

        emit ipAddressChanged(newIp);
    }
}

void TimingCamera::setConnectionStatus(QString status) {
    // Re-enable buttons if the status is disconnected
    if(status == "Disconnected") {
        reconnectButton->setEnabled(true);
        changeIpButton->setEnabled(true);
    } else {
        reconnectButton->setEnabled(false);
        changeIpButton->setEnabled(false);
    }
    serverStatus->setText(status);
}

void TimingCamera::startBackgroundThread() {
    // Start the separate thread and move the socket to it
    QThread *networkThread = new QThread();
    MyTcpSocket *socket = new MyTcpSocket(ipAddress->text(), directory);

    socket->moveToThread(networkThread);

    // Connect signals and slots
    connect(socket, SIGNAL(serverStatus(QString)), this, SLOT(setConnectionStatus(QString)));
    connect(socket, SIGNAL(newImage(QString)), this, SLOT(addNewImage(QString)));
    connect(networkThread, SIGNAL(started()), socket, SLOT(process()));
    connect(socket, SIGNAL(finished()), networkThread, SLOT(quit()));
    connect(socket, SIGNAL(finished()), socket, SLOT(deleteLater()));
    connect(networkThread, SIGNAL(finished()), networkThread, SLOT(deleteLater()));

    // Start the thread
    networkThread->start();
}

void TimingCamera::changeImage(int index) {
    // Account for the possibilty that we missed image(s)
    while(index >= entries.length()) {
        QString newName = QString("%1/1.png").arg(directory);
        if(entries.length() > 0) {
            bool ok;
            QFileInfo file(entries.at(entries.length() - 1).file);
            QString temp = file.baseName();
            qDebug("%s", temp.toLatin1().constData());
            int num = temp.toLong(&ok);
            if(ok)
                newName = QString("%1/%2.png").arg(directory).arg(num + 1);
        }
        QFile::copy(":images/images/No_image.png", newName);
        entries.append(Entry(newName, 0));
    }
    QImageReader reader(entries.at(index).file);
    reader.setAutoTransform(true);

    QImage image = reader.read();

    // Scale the image up if the width is smaller than 500px
    if(image.width() < 500)
        image = image.scaledToWidth(500);

    // Re-size the scroll area
    int newHeight = qRound((float)image.height()/(float)image.width()*500.0);
    scrollArea->setFixedHeight(newHeight + 12);

    actualImage->setPixmap(QPixmap::fromImage(image));
    scaleFactor = 1.0;

    actualImage->resize(500, newHeight);
}

void TimingCamera::addNewImage(QString fileName) {
    // Add the new image to the list of paths
    entries.append(Entry(fileName, 0));

    emit newImage();
}

void TimingCamera::zoomIn() {
    scaleImage(1.25);
}

void TimingCamera::zoomOut() {
    scaleImage(0.8);
}

void TimingCamera::scaleImage(double factor)
{
    scaleFactor *= factor;
    actualImage->resize(scaleFactor * QSize(scrollArea->width() - 12, scrollArea->height() - 12));

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    plusButton->setEnabled(scaleFactor < 3.0);
    minusButton->setEnabled(scaleFactor > 1.0);
}

void TimingCamera::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}
