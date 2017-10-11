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

#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QImage>
#include <QInputDialog>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QThread>
#include <QTimer>
#include <QTextStream>
#include <QVBoxLayout>

#include "timingcamera.h"
#include "timingpoint.h"

TimingPoint::TimingPoint(QString directory, QString name, QList<CameraInfo> cameras, int maxNum, int channelNum, QWidget *parent) : QGroupBox(parent)
{
    channel = channelNum;

    // Set the maximum number of times we can see someone before warning
    maxViews = maxNum;

    // Make sure we have a folder created for this timing point
    subDirectory = directory + name + "/";
    QDir *dir = new QDir(subDirectory);
    if(!dir->exists())
        dir->mkpath(subDirectory);
    delete dir;

    // Store the highest number of exisiting images for the slider
    int maxNumberOfImages = 0;

    for(int i = 0; i < cameras.length(); i++) {
        timingCameras.append(new TimingCamera(subDirectory + cameras.at(i).name, cameras.at(i).ip));
        TimingCamera *temp = timingCameras.last();
        temp->setAtBack(cameras.at(i).atBack);
        int numImages = temp->entries.length();
        if(numImages > maxNumberOfImages)
            maxNumberOfImages = numImages;
    }

    // Create our info labels
    QLabel *timestampLabel = new QLabel(this);
    timestampLabel->setText(tr("Timestamp:"));
    QLabel *bibNumLabel = new QLabel(this);
    bibNumLabel->setText(tr("Bib Number:"));

    // Initialize variable labels
    timestamp = new QLabel(this);

    // Initialize push buttons
    nextButton = new QPushButton(this);
    nextButton->setText(tr("Next Person"));

    // Initialize line edits
    bibNumEdit = new QLineEdit(this);

    // Slider to switch among images
    imageSlider = new QSlider(this);
    imageSlider->setMinimum(0);
    imageSlider->setMaximum(maxNumberOfImages - 1);
    imageSlider->setOrientation(Qt::Horizontal);
    imageSlider->setTracking(false);

    // Add buttons on each end of the slider
    QPushButton *minusButton = new QPushButton(this);
    minusButton->setIcon(QIcon(":/images/images/minus-icon.png"));
    minusButton->setIconSize(QSize(25, 25));

    QPushButton *plusButton = new QPushButton(this);
    plusButton->setIcon(QIcon(":/images/images/plus-icon.png"));
    plusButton->setIconSize(QSize(25, 25));

    // Settings button
    QPushButton *settingsButton = new QPushButton(this);
    settingsButton->setText("Point Settings");

    // Our layout
    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->setContentsMargins(3, 3, 3, 3);

    // Layouts of images and status boxes
    QScrollArea *imageHolderScroll = new QScrollArea(this);
    QScrollArea *statusBoxScroll = new QScrollArea(this);
    imageHolderScroll->setWidgetResizable(true);
    statusBoxScroll->setWidgetResizable(true);
    QWidget *imageHolderScrollWidget = new QWidget(this);
    QWidget *statusBoxScrollWidget = new QWidget(this);

    QHBoxLayout *imageHolderLayout = new QHBoxLayout(imageHolderScroll);
    QVBoxLayout *statusBoxLayout = new QVBoxLayout(statusBoxScroll);

    // Set widget of scroll area
    imageHolderScroll->setWidget(imageHolderScrollWidget);
    statusBoxScroll->setWidget(statusBoxScrollWidget);

    // Set layout of scroll area widget
    imageHolderScrollWidget->setLayout(imageHolderLayout);
    statusBoxScrollWidget->setLayout(statusBoxLayout);

    // Add images and status boxes to layouts
    for(int i = 0; i < timingCameras.length(); i++) {
        TimingCamera *temp = timingCameras.at(i);
        imageHolderLayout->addWidget(temp->imageHolder);
        statusBoxLayout->addWidget(temp->statusBox);
    }

    gridLayout->addWidget(imageHolderScroll, 0, 0, 7, 3);
    gridLayout->addWidget(statusBoxScroll, 6, 4, 1, 2);

    gridLayout->addWidget(timestampLabel, 0, 4, 1, 1);
    gridLayout->addWidget(timestamp, 0, 5, 1, 1);

    gridLayout->addWidget(bibNumLabel, 1, 4, 1, 1);
    gridLayout->addWidget(bibNumEdit, 1, 5, 1, 1);

    gridLayout->addWidget(nextButton, 2, 4, 1, 2);

    gridLayout->addWidget(settingsButton, 4, 4, 1, 2);

    gridLayout->addWidget(minusButton, 7, 0, 1, 1);
    gridLayout->addWidget(imageSlider, 7, 1, 1, 1);
    gridLayout->addWidget(plusButton, 7, 2, 1, 1);

    // Set stretch for the image
    gridLayout->setColumnStretch(1, 15);

    // Spacing on right-hand side
    gridLayout->setRowStretch(3, 1);
    gridLayout->setRowStretch(5, 1);

    setLayout(gridLayout);

    // Set the title of the timing point
    setTitle(name);

    // Button connections
    connect(nextButton, SIGNAL(clicked(bool)), this, SLOT(submitButtonPushed()));
    connect(plusButton, SIGNAL(clicked(bool)), this, SLOT(plusButtonPushed()));
    connect(minusButton, SIGNAL(clicked(bool)), this, SLOT(minusButtonPushed()));
    connect(settingsButton, SIGNAL(clicked(bool)), this, SLOT(changePointSettings()));

    // When pushing enter on the bib number, pretend the next button was pushed
    connect(bibNumEdit, SIGNAL(returnPressed()), this, SLOT(submitButtonPushed()));

    // Slider connection
    connect(imageSlider, SIGNAL(valueChanged(int)), this, SLOT(updateImageInfo(int)));

    // Connections to Cameras
    for(int i = 0; i < timingCameras.length(); i++) {
        TimingCamera *temp = timingCameras.at(i);
        connect(this, SIGNAL(changeImage(int)), temp, SLOT(changeImage(int)));
        connect(temp, SIGNAL(newImage()), this, SLOT(incrementSliderMax()));
        connect(temp, SIGNAL(settingsChanged()), this, SIGNAL(settingsChanged()));
    }

    // Trigger change to image
    imageSlider->triggerAction(QAbstractSlider::SliderToMaximum);

    // Create the csv file that we read from and write to
    csvFile = new QFile(subDirectory + "output.csv");

    // Create a hash table of all times and bibs so we don't double them
    if(csvFile->open(QFile::ReadOnly)) {
        QTextStream in(csvFile);
        QString line;
        QStringList list;
        while(!in.atEnd()) {
            line = in.readLine();
            list = line.split(',', QString::SkipEmptyParts);
            if (list.length() == 2) {
                hash.insert(list.at(0), list.at(1));
            }
        }
    }
    csvFile->close();
    csvFile->open(QFile::Append);
}

TimingPoint::~TimingPoint() {
    csvFile->close();
    delete csvFile;
}

void TimingPoint::submitButtonPushed() {
    bool write = true;

    // Write the time and bib number to the CSV
    QString bibNumber = bibNumEdit->text();

    // Divide up the bib numbers if there's multiple separated by a comma
    QStringList bibNums = bibNumber.split(',', QString::SkipEmptyParts);

    QTextStream out(csvFile);
    for(int i = 0; i < bibNums.length(); i++) {
        QString bibnum = bibNums.at(i);
        // Check if we've already written this bib number the maximum number of times
        int views = 0;
        for(int j = 0; j < bibsUsed.length(); j++) {
            if(bibsUsed.at(j) == bibnum)
                views += 1;
            if(views == maxViews) {
                // We've seen this bib the max number of times, make sure we want to continue
                QMessageBox *msgBox = new QMessageBox(this);
                msgBox->setText("This bib has already been entered the maximum number of times.  Continue?");
                msgBox->setStandardButtons(QMessageBox::Yes|QMessageBox::No);
                int ret = msgBox->exec();
                if(ret == QMessageBox::No) {
                    write = false;
                    delete msgBox;
                    return;
                } else {
                    delete msgBox;
                    break;
                }
            }
        }

        if(write) {
            QTime time = QTime::fromString(timestamp->text(), "hh:mm:ss.zzz");
            QString actualTime = roundTime(time, i);

            // Make sure we haven't tried using this time already
            QString prevBib = hash[actualTime];
            if(!prevBib.isEmpty()) {
                // We've used this timestamp, warn
                QMessageBox *msgBox = new QMessageBox(this);
                msgBox->setText(QString("This timestamp has already been used for bib %1.  Continue?").arg(prevBib));
                msgBox->setStandardButtons(QMessageBox::Yes|QMessageBox::No);
                int ret = msgBox->exec();
                if(ret == QMessageBox::No) {
                    delete msgBox;
                    return;
                } else {
                    delete msgBox;
                }
            }
            out << actualTime + "," + bibnum + "\n";

            // Add the timestamp and the bib number
            hash.insert(actualTime, bibnum);

            // Add this bid to the used bibs
            bibsUsed.append(bibnum);

            emit newEntry(channel, bibnum, actualTime);

            csvFile->flush();
        }
    }

    // Switch to the next image
    int temp = imageSlider->value() + 1;
    if (temp <= imageSlider->maximum()) {
        imageSlider->triggerAction(QAbstractSlider::SliderSingleStepAdd);
    }
}

void TimingPoint::plusButtonPushed() {
    int sliderPosition = imageSlider->sliderPosition();
    if(sliderPosition < imageSlider->maximum()) {
        imageSlider->triggerAction(QAbstractSlider::SliderSingleStepAdd);
    }
}

void TimingPoint::minusButtonPushed() {
    int sliderPosition = imageSlider->sliderPosition();
    if(sliderPosition >imageSlider->minimum()) {
        imageSlider->triggerAction(QAbstractSlider::SliderSingleStepSub);
    }
}

void TimingPoint::changePointSettings() {
    // Get connection info
    QDialog dialog;
    QFormLayout formLayout(&dialog);

    // Max views
    QLineEdit *maxViewsEdit = new QLineEdit(&dialog);
    maxViewsEdit->setText(QString("%1").arg(maxViews));
    QLabel maxViewsLabel(tr("Max Views:"));
    formLayout.addRow(&maxViewsLabel, maxViewsEdit);

    // Summit channel
    QLineEdit *summitChannel = new QLineEdit(&dialog);
    summitChannel->setText(QString("%1").arg(channel));
    QLabel summitChannelLabel(tr("Summit Channel:"));
    formLayout.addRow(&summitChannelLabel, summitChannel);

    // Buttons
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    formLayout.addRow(&buttonBox);

    // Make connections
    connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    if (dialog.exec() == QDialog::Accepted) {
        maxViews = maxViewsEdit->text().toInt();
        channel = summitChannel->text().toInt();
        emit settingsChanged();
    }
}

void TimingPoint::updateImageInfo(int index) {
    // Emit the signal to the cameras to change their image
    emit changeImage(index);

    bool timestampFound = false;
    int i = 0;
    int numCameras = timingCameras.length();
    while(!timestampFound && i < numCameras) {
        // Try updating the timestamp
        TimingCamera *tempCamera = timingCameras.at(i);
        if(index >= tempCamera->entries.length()) {
            i++;
            continue;
        }
        qint64 timestampValue = tempCamera->entries.at(index).timestamp;

        if(timestampValue != 0) {
            //Convert the raw timestamp to user-readable string
            QDateTime time;
            time = QDateTime::fromMSecsSinceEpoch(timestampValue);
            timestamp->setText(time.time().toString("hh:mm:ss.zzz"));
            // Enable the next button
            nextButton->setEnabled(true);
            timestampFound = true;
            break;
        } else {
            i++;
        }
    }

    if(!timestampFound) {
        timestamp->setText("");
        nextButton->setEnabled(false);
    }

    // If we've already written a bib number, show it in the Line Edit
    bibNumEdit->setText(hash[timestamp->text()]);

    // If we didn't find a number, check if we received one from the camera
    if(bibNumEdit->text().isEmpty() && timingCameras.length() > 0) {
        bool allMatch = true;
        TimingCamera *temp = timingCameras.at(0);
        int firstBibNumber = temp->entries.back().bibNumber;
        i = 1;
        while(allMatch && i < numCameras) {
            temp = timingCameras.at(i);
            if(index >= temp->entries.length()) {
                i++;
                continue;
            }
            int number = temp->entries.at(index).bibNumber;
            if(number == 0 || number != firstBibNumber) {
                // Mismatch
                allMatch = false;
            } else {
                i++;
            }
        }

        if(allMatch) {
            bibNumEdit->setText(QString("%1").arg(firstBibNumber));
            submitButtonPushed();
        }
    }

    // Change the focus
    bibNumEdit->setFocus();
}

QString TimingPoint::roundTime(QTime time, int nth) {
    // Convert time to hundreths, for nth bib add .1s for each
    int dsecs = qRound((double)time.msec()/10.0) + 10*nth;
    while(dsecs >= 100) {
        dsecs -= 100;
        time = time.addMSecs(1000);
    }
    QString actualTime((time.toString("hh:mm:ss") + ".%1").arg(dsecs));
    return actualTime;
}

QList<TimingPoint::CameraInfo> TimingPoint::getCameraInfo() {
    // Save the IPs to a file
    QList<CameraInfo> info;
    for(int i = 0; i < timingCameras.length(); i++) {
        TimingCamera *temp = timingCameras.at(i);
        info.append(CameraInfo(temp->getName(), temp->getIpAddress(), temp->getAtBack()));
    }
    return info;
}

QString TimingPoint::getTitle() {
    return title();
}

int TimingPoint::getChannel() {
    return channel;
}

int TimingPoint::getMaxViews() {
    return maxViews;
}

void TimingPoint::incrementSliderMax() {
    int numCameras = timingCameras.length();
    if(numCameras == 0) {
        imageSlider->setMaximum(0);
    } else {
        bool allMatch = true;
        TimingCamera *temp = timingCameras.at(0);
        int maxEntries = temp->entries.length();
        qint64 biggestTimestamp = temp->entries.back().timestamp;
        qint64 biggestGap = 0;
        for(int i = 1; i < numCameras; i++) {
            temp = timingCameras.at(i);
            int length = temp->entries.length();
            if(length > maxEntries) {
                maxEntries = length;
                allMatch = false;
            } else if(length < maxEntries)
                allMatch = false;
            qint64 currentTimestamp = temp->entries.back().timestamp;
            if(currentTimestamp > biggestTimestamp)
                biggestTimestamp = currentTimestamp;
            qint64 currentGap = abs(biggestTimestamp - currentTimestamp);
            if(currentGap > biggestGap)
                biggestGap = currentGap;
        }

        if(allMatch) {
            imageSlider->setMaximum(maxEntries - 1);
        } else {
            if(biggestGap > 15000) {
                // Gap detected, sync up number of images
                for(int i = 0; i < numCameras; i++) {
                    temp = timingCameras.at(i);
                    while(temp->entries.length() < maxEntries)
                        temp->addBlankImage(biggestTimestamp);
                }
            }
            imageSlider->setMaximum(maxEntries - 1);
        }
        // Trigger the slider move, so if we were showing an old image,the new one will appear
        imageSlider->triggerAction(QAbstractSlider::SliderMove);
    }
}
