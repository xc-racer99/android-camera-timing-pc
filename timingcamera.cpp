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

#include <QCheckBox>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QImageReader>
#include <QInputDialog>
#include <QLineEdit>
#include <QPainter>
#include <QScrollArea>
#include <QSlider>
#include <QThread>
#include <QTimer>

#include "mytcpsocket.h"

#include "timingcamera.h"

TimingCamera::TimingCamera(QString dir, QString ip, QObject *parent) : QObject(parent)
{
    directory = dir + "/";

    scaleFactor = 1.0;

    fromBack = false;

    timeOffset = 0;

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
    serverStatusLabel->setText(tr("Server:"));

    // Initialize the variable labels
    ipAddress = new QLabel(statusBox);
    ipAddress->setText(ip);
    serverStatus = new QLabel(statusBox);

    // Initialize push buttons
    reconnectButton = new QPushButton(statusBox);
    reconnectButton->setText(tr("Connect"));
    changeSettingsButton = new QPushButton(statusBox);
    changeSettingsButton->setText(tr("Settings"));

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

    // Add a blank image
    entries.append(Entry(":/images/images/No_image.png", 0, 0));

    statusLayout->addWidget(ipAddressLabel, 0, 0, 1, 1);
    statusLayout->addWidget(ipAddress, 0, 1, 1, 1);
    statusLayout->addWidget(serverStatusLabel, 1, 0, 1, 1);
    statusLayout->addWidget(serverStatus, 1, 1, 1, 1);
    statusLayout->addWidget(reconnectButton, 2, 0, 1, 1);
    statusLayout->addWidget(changeSettingsButton, 2, 1, 1, 1);
    statusBox->setLayout(statusLayout);

    // Button connections
    connect(reconnectButton, SIGNAL(clicked(bool)), this, SLOT(reconnectToServer()));
    connect(changeSettingsButton, SIGNAL(clicked(bool)), this, SLOT(changeSettings()));
    connect(minusButton, SIGNAL(clicked(bool)), this, SLOT(zoomOut()));
    connect(plusButton, SIGNAL(clicked(bool)), this, SLOT(zoomIn()));

    // Create a list of images already present
    QStringList filter("*.jpg");
    QFileInfoList initialFileInfo = temp.entryInfoList(filter, QDir::Files, QDir::Name);
    for(int i = 0; i < initialFileInfo.length(); i++) {
        QString rawTimestamp = initialFileInfo.at(i).baseName();
        qint64 timestamp = rawTimestamp.toLongLong();
        entries.append(Entry(initialFileInfo.at(i).absoluteFilePath(), 0, timestamp));
    }

    // Start the OCR
    pipeline = new OcrPipeline(directory, fromBack);
    QThread *thread = new QThread();
    pipeline->moveToThread(thread);

    connect(thread, SIGNAL(started()), pipeline, SLOT(process()));
    connect(pipeline, SIGNAL(finished()), thread, SLOT(quit()));
    connect(pipeline, SIGNAL(finished()), pipeline, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(pipeline, SIGNAL(newImage(QString,int)), this, SLOT(addNewImage(QString,int)));

    thread->start();

    // Start the image saving thread
    startBackgroundThread();
}

TimingCamera::~TimingCamera() {
    delete ipAddress;
    delete statusBox;
    delete imageHolder;
    pipeline->stopThread();
}

void TimingCamera::addBlankImage(qint64 time) {
    QString newName = QString("%1/%2.png").arg(directory).arg(time);
    QFile::copy(":images/images/No_image.png", newName);
    entries.append(Entry(newName, 0, 0));
}

void TimingCamera::setAtBack(bool fromBehind) {
    fromBack = fromBehind;
    pipeline->setFromBehind(fromBehind);
}

void TimingCamera::setTimestampOffset(qint64 offset) {
    timeOffset = offset;
}

bool TimingCamera::getAtBack() {
    return fromBack;
}

QString TimingCamera::getIpAddress() {
    return ipAddress->text();
}

QString TimingCamera::getName() {
    QDir info(directory);
    return info.dirName();
}

qint64 TimingCamera::getTimestampOffset() {
    return timeOffset;
}

void TimingCamera::reconnectToServer() {
    bool ok;
    QString temp = QInputDialog::getText(0, tr("Choose IP and Connect"), tr("IP Address:"), QLineEdit::Normal, ipAddress->text(), &ok);
    if(ok && !temp.isEmpty()) {
        ipAddress->setText(temp);
        emit settingsChanged();
        startBackgroundThread();
    }
}

void TimingCamera::changeSettings() {
    QDialog *dialog = new QDialog();
    QFormLayout *layout = new QFormLayout(dialog);

    QLabel *offsetLabel = new QLabel(dialog);
    offsetLabel->setText(tr("Time Offset (ms):"));
    QLineEdit *offsetLineEdit = new QLineEdit(dialog);
    offsetLineEdit->setText(QString("%1").arg(timeOffset));
    layout->addRow(offsetLabel, offsetLineEdit);

    QCheckBox *atBack = new QCheckBox(dialog);
    atBack->setText(tr("From Behind?"));
    atBack->setChecked(fromBack);
    layout->addRow(atBack);

    // Buttons
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, dialog);
    layout->addRow(&buttonBox);

    dialog->setLayout(layout);

    // Make connections
    connect(&buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
    connect(&buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));

    if(dialog->exec() == QDialog::Accepted) {
        setAtBack(atBack->checkState());
        setTimestampOffset(offsetLineEdit->text().toLongLong());
    }
    emit settingsChanged();
}

void TimingCamera::setConnectionStatus(QString status) {
    // Re-enable buttons if the status is disconnected
    if(status == "Disconnected") {
        reconnectButton->setEnabled(true);
    } else {
        reconnectButton->setEnabled(false);
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
    connect(socket, SIGNAL(newImage(QString)), pipeline, SLOT(addImage(QString)));
    connect(networkThread, SIGNAL(started()), socket, SLOT(process()));
    connect(socket, SIGNAL(finished()), networkThread, SLOT(quit()));
    connect(socket, SIGNAL(finished()), socket, SLOT(deleteLater()));
    connect(networkThread, SIGNAL(finished()), networkThread, SLOT(deleteLater()));

    // Start the thread
    networkThread->start();
}

void TimingCamera::changeImage(int index) {
    if(index > entries.length() - 1) {
        qDebug("Warning: Index is greater than permissible");
        index = 0;
    }

    QImageReader reader(entries.at(index).file);
    reader.setAutoTransform(true);

    QImage image = reader.read();

    // Scale the image up if the width is smaller than 500px
    if(image.width() < 500)
        image = image.scaledToWidth(500);

    // Overlay the timestamp on the image
    QDateTime time = QDateTime::fromMSecsSinceEpoch(entries.at(index).timestamp);
    QPainter *painter = new QPainter(&image);
    painter->setPen(Qt::blue);
    painter->setFont(QFont("Arial", 100));
    painter->drawText(image.rect(), Qt::AlignLeft|Qt::AlignTop, time.time().toString("hh:mm:ss.zzz"));
    delete(painter);

    // Re-size the scroll area
    int newHeight = qRound((float)image.height()/(float)image.width()*500.0);
    scrollArea->setFixedHeight(newHeight + 12);

    actualImage->setPixmap(QPixmap::fromImage(image));
    scaleFactor = 1.0;

    actualImage->resize(500, newHeight);
}

void TimingCamera::addNewImage(QString fileName, int bibNumber) {
    // Add the new image to the list of paths
    QFileInfo info(fileName);
    QString rawTimestamp = info.baseName();
    bool ok;
    qint64 timestamp = rawTimestamp.toLongLong(&ok);
    if(ok) {
        // Check the last entry and see if it was a dummy
        QFileInfo tempFile = QFile(entries.back().file);
        qint64 lastActualTimestamp = tempFile.baseName().toLongLong();
        if(entries.back().timestamp == 0 && lastActualTimestamp - timestamp < 2000 && lastActualTimestamp - timestamp > -2000)
            entries.replace(entries.length() - 1, Entry(fileName, bibNumber, timestamp + timeOffset));
        else
            entries.append(Entry(fileName, bibNumber, timestamp + timeOffset));
    } else
        entries.append(Entry(fileName, bibNumber, 0));

    // Create a delay of 1s to hopefully give time for the other thread(s) to finish if they're still going
    QTimer::singleShot(1000, this, SIGNAL(newImage()));
}

void TimingCamera::checkEntries(int numEntries, qint64 timestamp) {
    if(numEntries == entries.length()) {
        // All good, all cameras are in sync
        return;
    }
    if(entries.length() > numEntries)
        qDebug("Error: Number of entries here is greater than greatest number of entries.  This shouldn't be possible");

    // Add extra dummy images as needed
    for(int numDiff = numEntries - entries.length();numDiff > 0; numDiff--) {
        addBlankImage(timestamp - numDiff);
    }
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
