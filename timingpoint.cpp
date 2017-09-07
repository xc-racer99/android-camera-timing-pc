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

#include "mytcpsocket.h"
#include "timingpoint.h"

TimingPoint::TimingPoint(QString directory, QString name, QString ip, QWidget *parent) : QGroupBox(parent)
{
    // Make sure we have a folder created for this timing point
    subDirectory = directory + name + "/";
    QDir *dir = new QDir(subDirectory);
    if(!dir->exists())
        dir->mkpath(subDirectory);

    // Create our info labels
    QLabel *ipAddressLabel = new QLabel(this);
    ipAddressLabel->setText(tr("IP:"));
    QLabel *serverStatusLabel = new QLabel(this);
    serverStatusLabel->setText(tr("Server Status:"));
    QLabel *timestampLabel = new QLabel(this);
    timestampLabel->setText(tr("Timestamp:"));
    QLabel *bibNumLabel = new QLabel(this);
    bibNumLabel->setText(tr("Bib Number:"));

    // Initialize variable labels
    timestamp = new QLabel(this);
    ipAddress = new QLabel(this);
    ipAddress->setText(ip);
    serverStatus = new QLabel(this);

    // Initialize push buttons
    nextButton = new QPushButton(this);
    nextButton->setText(tr("Next Person"));
    reconnectButton = new QPushButton(this);
    reconnectButton->setText(tr("Reconnect"));
    changeIpButton = new QPushButton();
    changeIpButton->setText(tr("Change IP"));

    // Initialize line edits
    bibNumEdit = new QLineEdit(this);

    // Setup a QLabel which holds the image
    imageHolder = new QLabel(this);
    imageHolder->setBackgroundRole(QPalette::Base);
    imageHolder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    imageHolder->setScaledContents(true);

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

    // Choose our blank image
    imagePaths.append(":/images/images/No_image.png");
    changeImage(0);

    // Our layout
    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->setContentsMargins(5, 5, 5, 5);

    gridLayout->addWidget(plusButton, 6, 2, 1, 1);
    gridLayout->addWidget(timestampLabel, 0, 4, 1, 1);
    gridLayout->addWidget(ipAddressLabel, 4, 4, 1, 1);
    gridLayout->addWidget(ipAddress, 4, 5, 1, 1);
    gridLayout->addWidget(bibNumEdit, 1, 5, 1, 1);
    gridLayout->addWidget(serverStatusLabel, 5, 4, 1, 1);
    gridLayout->addWidget(serverStatus, 5, 5, 1, 1);
    gridLayout->addWidget(nextButton, 2, 4, 1, 2);
    gridLayout->addWidget(bibNumLabel, 1, 4, 1, 1);
    gridLayout->addWidget(imageSlider, 6, 1, 1, 1);
    gridLayout->addWidget(timestamp, 0, 5, 1, 1);
    gridLayout->addWidget(minusButton, 6, 0, 1, 1);
    gridLayout->addWidget(imageHolder, 0, 0, 6, 3);
    gridLayout->addWidget(reconnectButton, 6, 4, 1, 1);
    gridLayout->addWidget(changeIpButton, 6, 5, 1, 1);

    gridLayout->setRowStretch(3, 10);
    gridLayout->setColumnStretch(3, 10);

    setLayout(gridLayout);

    // Set the title of the timing point
    setTitle(name);

    // Button connections
    connect(reconnectButton, SIGNAL(clicked(bool)), this, SLOT(reconnectToServer()));
    connect(changeIpButton, SIGNAL(clicked(bool)), this, SLOT(changeIpDialog()));
    connect(nextButton, SIGNAL(clicked(bool)), this, SLOT(submitButtonPushed()));
    connect(plusButton, SIGNAL(clicked(bool)), this, SLOT(plusButtonPushed()));
    connect(minusButton, SIGNAL(clicked(bool)), this, SLOT(minusButtonPushed()));

    // When pushing enter on the bib number, pretend the next button was pushed
    connect(bibNumEdit, SIGNAL(returnPressed()), this, SLOT(submitButtonPushed()));

    // Slider connection
    connect(imageSlider, SIGNAL(valueChanged(int)), this, SLOT(changeImage(int)));

    // Save the settings
    saveSettings();

    // Create a list of images already present
    QDir subDir(subDirectory);
    QStringList filter("*.jpg");
    QFileInfoList initialFileInfo = subDir.entryInfoList(filter, QDir::Files, QDir::Name);
    for(int i = 0; i < initialFileInfo.length(); i++)
        imagePaths.append(initialFileInfo.at(i).absoluteFilePath());

    // Set the initial image if we already have images
    if(imagePaths.length() > 1) {
        // Set the image slider length to the number of images we have
        imageSlider->setMaximum(imagePaths.length() - 1);
        imageSlider->setSliderPosition(imagePaths.length() - 1);
        changeImage(imagePaths.length() - 1);
    }

    // Create the csv file that we read from and write to
    csvFile = new QFile(subDirectory + "output.csv");
    csvFile->open(QFile::Append | QFile::ReadOnly);

    // Start the image saving thread
    startBackgroundThread(ip, subDirectory);
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

void TimingPoint::setIpAddress(QString newIp) {
    ipAddress->setText(newIp);
}

void TimingPoint::setConnectionStatus(QString status) {
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

void TimingPoint::changeIpDialog() {
    bool ok;
    QString newIp = QInputDialog::getText(this, tr("Enter new IP"), tr("IP:"), QLineEdit::Normal, ipAddress->text(), &ok);

    // Update the ip address shown
    if(ok && !newIp.isEmpty()) {
        ipAddress->setText(newIp);

        saveSettings();
    }
}

void TimingPoint::reconnectToServer() {
    startBackgroundThread(ipAddress->text(), subDirectory);
}

void TimingPoint::changeImage(int index) {
    QImage image(imagePaths.at(index));
    QImage scaledImage = image.scaledToHeight(512);
    imageHolder->setPixmap(QPixmap::fromImage(scaledImage));
    imageHolder->setMaximumSize(scaledImage.width(), scaledImage.height());

    // Update the timestamp
    QFileInfo pic = imagePaths.at(index);
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
        timestamp->setText("");
        // Disable the next button
        nextButton->setEnabled(false);
    }

    // If we've already written a bib number, show it in the Line Edit
    bibNumEdit->setText("");

    // Change the focus
    bibNumEdit->setFocus();
}

void TimingPoint::saveSettings() {
    // Save the IP to a file
    QFile settingsFile(subDirectory + ".settings");
    settingsFile.open(QIODevice::WriteOnly);
    QTextStream out(&settingsFile);
    out << ipAddress->text();
    settingsFile.flush();
    settingsFile.close();
}

void TimingPoint::startBackgroundThread(QString ip, QString subDir) {
    // Start the separate thread and move the socket to it
    QThread *networkThread = new QThread();
    MyTcpSocket *socket = new MyTcpSocket(ip, subDir);

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

void TimingPoint::addNewImage(QString fileName) {
    // Add the new image to the list of paths
    imagePaths.append(fileName);

    // Increment the slider's maximum
    imageSlider->setMaximum(imageSlider->maximum() + 1);

    // Change the image if this is the first one
    if(imagePaths.length() == 2) {
        imageSlider->triggerAction(QAbstractSlider::SliderToMaximum);
    }
}
