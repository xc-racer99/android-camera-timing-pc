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
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QImage>
#include <QInputDialog>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
#include <QThread>
#include <QTimer>
#include <QTextStream>

#include "timingcamera.h"
#include "timingpoint.h"

TimingPoint::TimingPoint(QString directory, QString name, QString ip, QString secondIp, int maxNum, int channelNum, QWidget *parent) : QGroupBox(parent)
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

    // Create our camera devices
    mainCamera = new TimingCamera(subDirectory + "Main/", ip, this);
    secondCamera = new TimingCamera(subDirectory + "Secondary/", secondIp, this);

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
    imageSlider->setMaximum(0);
    imageSlider->setOrientation(Qt::Horizontal);
    imageSlider->setTracking(false);

    // Add buttons on each end of the slider
    QPushButton *minusButton = new QPushButton(this);
    minusButton->setIcon(QIcon(":/images/images/minus-icon.png"));
    minusButton->setIconSize(QSize(25, 25));

    QPushButton *plusButton = new QPushButton(this);
    plusButton->setIcon(QIcon(":/images/images/plus-icon.png"));
    plusButton->setIconSize(QSize(25, 25));

    // Our layout
    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->setContentsMargins(5, 5, 5, 5);

    gridLayout->addWidget(plusButton, 6, 3, 1, 1);
    gridLayout->addWidget(timestampLabel, 0, 5, 1, 1);
    gridLayout->addWidget(bibNumEdit, 1, 6, 1, 1);
    gridLayout->addWidget(nextButton, 2, 5, 1, 2);
    gridLayout->addWidget(bibNumLabel, 1, 5, 1, 1);
    gridLayout->addWidget(imageSlider, 6, 1, 1, 2);
    gridLayout->addWidget(timestamp, 0, 6, 1, 1);
    gridLayout->addWidget(minusButton, 6, 0, 1, 1);
    gridLayout->addWidget(mainCamera->imageHolder, 0, 0, 6, 2);
    gridLayout->addWidget(mainCamera->statusBox, 4, 5, 1, 2);

    gridLayout->addWidget(secondCamera->imageHolder, 0, 2, 6, 2);
    gridLayout->addWidget(secondCamera->statusBox, 5, 5, 2, 2);

    // Set stretch for the parts of the image
    gridLayout->setColumnStretch(1, 5);
    gridLayout->setColumnStretch(2, 5);

    gridLayout->setRowStretch(3, 10);
    gridLayout->setColumnStretch(4, 10);

    setLayout(gridLayout);

    // Set the title of the timing point
    setTitle(name);

    // Button connections
    connect(nextButton, SIGNAL(clicked(bool)), this, SLOT(submitButtonPushed()));
    connect(plusButton, SIGNAL(clicked(bool)), this, SLOT(plusButtonPushed()));
    connect(minusButton, SIGNAL(clicked(bool)), this, SLOT(minusButtonPushed()));

    // When pushing enter on the bib number, pretend the next button was pushed
    connect(bibNumEdit, SIGNAL(returnPressed()), this, SLOT(submitButtonPushed()));

    // Slider connection
    connect(imageSlider, SIGNAL(valueChanged(int)), this, SLOT(updateImageInfo(int)));

    // Connections to Camera
    connect(this, SIGNAL(changeImage(int)), mainCamera, SLOT(changeImage(int)));
    connect(mainCamera, SIGNAL(newImage()), this, SLOT(incrementSliderMax()));
    connect(mainCamera, SIGNAL(settingsChanged(QString)), this, SLOT(saveSettings()));

    connect(this, SIGNAL(changeImage(int)), secondCamera, SLOT(changeImage(int)));
    connect(secondCamera, SIGNAL(newImage()), this, SLOT(incrementSliderMax()));
    connect(secondCamera, SIGNAL(settingsChanged(QString)), this, SLOT(saveSettings()));

    // Save the settings
    saveSettings();

    // Trigger change to image
    imageSlider->triggerAction(QAbstractSlider::SliderToMinimum);

    // Set the initial image if we already have images
    if(mainCamera->entries.length() > 1) {
        // Set the image slider length to the number of images we have
        imageSlider->setMaximum(mainCamera->entries.length() - 1);
        imageSlider->triggerAction(QAbstractSlider::SliderToMaximum);
    }

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

void TimingPoint::updateImageInfo(int index) {
    // Emit the signal to the cameras to change their image
    emit changeImage(index);

    // Update the timestamp
    QFileInfo pic = mainCamera->entries.at(index).file;
    QString rawTimestamp = pic.baseName();

    //Convert the raw timestamp to user-readable string
    bool ok;
    QDateTime time;
    qint64 temp = rawTimestamp.toLongLong(&ok);
    if(ok) {
        time = QDateTime::fromMSecsSinceEpoch(temp);
        timestamp->setText(time.time().toString("hh:mm:ss.zzz"));
        // Enable the next button
        nextButton->setEnabled(true);
    } else {
        // Presumably there was no image at the main camera, so try getting the timestamp from the second image
        pic = secondCamera->entries.at(index).file;
        rawTimestamp = pic.baseName();
        temp = rawTimestamp.toLongLong(&ok);
        if(ok) {
            time = QDateTime::fromMSecsSinceEpoch(temp);
            timestamp->setText(time.time().toString("hh:mm:ss.zzz"));
            // Enable the next button
            nextButton->setEnabled(true);
        } else {
            timestamp->setText("");
            // Disable the next button
            nextButton->setEnabled(false);
        }
    }

    // If we've already written a bib number, show it in the Line Edit
    bibNumEdit->setText(hash[roundTime(time.time(), 0)]);

    // If we didn't find a number, check if we received one from the camera
    if(bibNumEdit->text().isEmpty()) {
        int number = mainCamera->entries.at(index).bibNumber;
        if(number != 0 && number == secondCamera->entries.at(index).bibNumber) {
            bibNumEdit->setText(QString("%1").arg(number));
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

void TimingPoint::saveSettings() {
    // Save the IPs to a file
    QFile settingsFile(subDirectory + ".settings");
    settingsFile.open(QIODevice::WriteOnly);
    QTextStream out(&settingsFile);
    out << mainCamera->ipAddress->text() + "\n" + secondCamera->ipAddress->text()
        + "\n" + maxViews + "\n" + channel;
    settingsFile.flush();
    settingsFile.close();
}

void TimingPoint::incrementSliderMax() {
    int mainImages = mainCamera->entries.length();
    int secondImages = secondCamera->entries.length();
    sliderMax = qMax(mainImages, secondImages) - 1;
    if(mainImages != secondImages) {
        // Delay 1/2 a second to hopefully get things in sync before incrementing the slider
        QTimer::singleShot(750, this, SLOT(doIncrementSliderMax()));
    } else {
        doIncrementSliderMax();
    }
}

void TimingPoint::doIncrementSliderMax() {
    imageSlider->setMaximum(sliderMax);
}
