#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QImage>
#include <QInputDialog>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
#include <QThread>
#include <QTextStream>

#include "timingcamera.h"
#include "timingpoint.h"

TimingPoint::TimingPoint(QString directory, QString name, QString ip, QString secondIp, QWidget *parent) : QGroupBox(parent)
{
    // Make sure we have a folder created for this timing point
    subDirectory = directory + name + "/";
    QDir *dir = new QDir(subDirectory);
    if(!dir->exists())
        dir->mkpath(subDirectory);

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
    connect(mainCamera, SIGNAL(ipAddressChanged(QString)), this, SLOT(saveSettings()));

    connect(this, SIGNAL(changeImage(int)), secondCamera, SLOT(changeImage(int)));
    connect(secondCamera, SIGNAL(newImage()), this, SLOT(incrementSliderMax()));
    connect(secondCamera, SIGNAL(ipAddressChanged(QString)), this, SLOT(saveSettings()));

    // Save the settings
    saveSettings();

    // Trigger change to image
    imageSlider->triggerAction(QAbstractSlider::SliderToMinimum);

    // Set the initial image if we already have images
    if(mainCamera->imagePaths.length() > 1) {
        // Set the image slider length to the number of images we have
        imageSlider->setMaximum(mainCamera->imagePaths.length() - 1);
        imageSlider->triggerAction(QAbstractSlider::SliderToMaximum);
    }

    // Create the csv file that we read from and write to
    csvFile = new QFile(subDirectory + "output.csv");
    csvFile->open(QFile::Append | QFile::ReadOnly);
}

void TimingPoint::submitButtonPushed() {
    // Write the time and bib number to the CSV
    QString time = timestamp->text();
    QString bibNumber = bibNumEdit->text();

    // Divide up the bib numbers if there's multiple separated by a comma
    QStringList bibNums = bibNumber.split(',', QString::SkipEmptyParts);

    QTextStream out(csvFile);
    for(int i = 0; i < bibNums.length(); i++) {
        out << time + "," + bibNums.at(i) + "\n";
        csvFile->flush();
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
    // Add a slight delay in case one of the network connections is lagging slightly
//    msleep(250);

    // Emit the signal to the cameras to change their image
    emit changeImage(index);

    // Update the timestamp
    QFileInfo pic = mainCamera->imagePaths.at(index);
    QString rawTimestamp = pic.baseName();

    //Convert the raw timestamp to user-readable string
    bool ok;
    qint64 temp = rawTimestamp.toLongLong(&ok);
    if(ok) {
        QDateTime time = QDateTime::fromMSecsSinceEpoch(temp);
        timestamp->setText(time.time().toString("hh:mm:ss.zzz"));
        // Enable the next button
        nextButton->setEnabled(true);
    } else {
        // Presumably there was no image at the main camera, so try getting the timestamp from the second image
        pic = secondCamera->imagePaths.at(index);
        rawTimestamp = pic.baseName();
        temp = rawTimestamp.toLongLong(&ok);
        if(ok) {
            QDateTime time = QDateTime::fromMSecsSinceEpoch(temp);
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
    bibNumEdit->setText("");

    // Change the focus
    bibNumEdit->setFocus();
}

void TimingPoint::saveSettings() {
    // Save the IPs to a file
    QFile settingsFile(subDirectory + ".settings");
    settingsFile.open(QIODevice::WriteOnly);
    QTextStream out(&settingsFile);
    out << mainCamera->ipAddress->text() + "\n" + secondCamera->ipAddress->text();
    settingsFile.flush();
    settingsFile.close();
}

void TimingPoint::incrementSliderMax() {
    int mainImages = mainCamera->imagePaths.length();
    int secondImages = mainCamera->imagePaths.length();
    imageSlider->setMaximum(qMax(mainImages, secondImages) - 1);
}
