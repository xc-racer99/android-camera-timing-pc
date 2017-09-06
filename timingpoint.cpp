#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QImage>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
#include <QThread>

#include "mytcpsocket.h"
#include "timingpoint.h"
#include "timingpointinfo.h"

TimingPoint::TimingPoint(QString directory, QString name, QString ip, QWidget *parent) : QGroupBox(parent)
{
    commonSetupCode(directory);
    setConnectionInfo(ip, name);
}

TimingPoint::TimingPoint(QString directory, QWidget *parent) : QGroupBox(parent)
{
    commonSetupCode(directory);

    // Create a dialog asking for connection info
    dialog = new QDialog(this);
    dialog->setFixedSize(210, 110);
    TimingPointInfo info(dialog);

    // Connect things together
    connect(&info, SIGNAL(setupCompleted(QString, QString)), this, SLOT(gotConnectionInfo(QString,QString)));
    connect(dialog, SIGNAL(rejected()), this, SLOT(noConnectionInfo()));

    // Show the dialog
    dialog->exec();
}

void TimingPoint::commonSetupCode(QString directory) {
    // Pass the directory along
    mainFolder = directory;

    // Create our strings
    ipAddressString = "0.0.0.0";

    // Setup a QLabel which holds the image
    imageHolder = new QLabel;
    imageHolder->setBackgroundRole(QPalette::Base);
    imageHolder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    imageHolder->setScaledContents(true);

    // Scroll bar to switch among images
    imageSlider = new QSlider();
    imageSlider->setMinimum(0);
    imageSlider->setMaximum(0);
    imageSlider->setOrientation(Qt::Horizontal);
    imageSlider->setTracking(false);

    // Add buttons
    nextButton = new QPushButton();
    nextButton->setText(tr("Next Person"));
    reconnectButton = new QPushButton();
    reconnectButton->setText(tr("Reconnect"));
    reconnectButton->setEnabled(false);
    changeIpButton = new QPushButton();
    changeIpButton->setText(tr("Change IP"));
    changeIpButton->setEnabled(false);

    // Add line edit for bib number
    bibNumEdit = new QLineEdit();
    QLabel *bibNumLabel = new QLabel();
    bibNumLabel->setText(tr("Bib Number:"));
    bibNumLabel->setBuddy(bibNumEdit);

    // Add info labels
    timestampLabel = new QLabel(tr("Timstamp:"));
    actualTimestamp = new QLabel("");
    ipAddressLabel = new QLabel();
    serverStatus = new QLabel("Server Status: Disconnected");

    // Choose our blank image
    imagePaths.append(":/images/images/No_image.png");
    changeImage(0);

    // Our layout
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setContentsMargins(5, 5, 5, 5);
    gridLayout->addWidget(ipAddressLabel, 4, 2, 1, 2);
    gridLayout->addWidget(timestampLabel, 0, 2, 1, 1);
    gridLayout->addWidget(reconnectButton, 6, 2, 1, 1);
    gridLayout->addWidget(changeIpButton, 6, 3, 1, 1);
    gridLayout->addWidget(bibNumEdit, 1, 3, 1, 1);
    gridLayout->addWidget(serverStatus, 5, 2, 1, 2);
    gridLayout->addWidget(nextButton, 2, 2, 1, 2);
    gridLayout->addWidget(imageHolder, 0, 0, 6, 1);
    gridLayout->addWidget(bibNumLabel, 1, 2, 1, 1);
    gridLayout->addWidget(imageSlider, 6, 0, 1, 1);
    gridLayout->addWidget(actualTimestamp, 0, 3, 1, 1);
    gridLayout->setRowStretch(3, 10);
    gridLayout->setColumnStretch(1, 10);

    setLayout(gridLayout);

    // Button connections
    connect(reconnectButton, SIGNAL(clicked(bool)), this, SLOT(reconnectToServer()));
    connect(changeIpButton, SIGNAL(clicked(bool)), this, SLOT(changeIpDialog()));
    connect(nextButton, SIGNAL(clicked(bool)), this, SLOT(submitButtonPushed()));

    // When pushing enter on the bib number, pretend the next button was pushed
    connect(bibNumEdit, SIGNAL(returnPressed()), this, SLOT(submitButtonPushed()));

    // Slider connection
    connect(imageSlider, SIGNAL(valueChanged(int)), this, SLOT(changeImage(int)));
}

void TimingPoint::submitButtonPushed() {
    // Write the time and bib number to the CSV
    QString time = actualTimestamp->text();
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
        changeImage(temp);
        imageSlider->setValue(temp);
    }
}

void TimingPoint::setIpAddress() {
    ipAddressLabel->setText(tr("IP: %1").arg(ipAddressString));
}

void TimingPoint::setConnected() {
    serverStatus->setText("Server Status: Connected");
    reconnectButton->setEnabled(false);
    changeIpButton->setEnabled(false);
}

void TimingPoint::setDisconnected() {
    serverStatus->setText("Server Status: Disconnected");
    reconnectButton->setEnabled(true);
    changeIpButton->setEnabled(true);
}

void TimingPoint::changeIpDialog() {
    bool ok;
    QString newIp = QInputDialog::getText(this, tr("Enter new IP"), tr("IP:"), QLineEdit::Normal, ipAddressString, &ok);

    // Update the ip address shown
    if(ok && !newIp.isEmpty()) {
        ipAddressString = newIp;
        setIpAddress();

        // Modify the .settings file
        QFile settingsFile(subDirectory + ".settings");
        settingsFile.open(QIODevice::WriteOnly);
        QTextStream out(&settingsFile);
        out << ipAddressString;
        settingsFile.flush();
        settingsFile.close();
    }
}

void TimingPoint::reconnectToServer() {
    startBackgroundThread(ipAddressString, subDirectory);
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
        actualTimestamp->setText(time.time().toString());
        // Enable the next button
        nextButton->setEnabled(true);
    } else {
        actualTimestamp->setText("");
        // Disable the next button
        nextButton->setEnabled(false);
    }

    // If we've already written a bib number, show it in the Line Edit
    bibNumEdit->setText("");

    // Change the focus
    bibNumEdit->setFocus();
}

void TimingPoint::gotConnectionInfo(QString ip, QString name) {
    // Close dialog box
    dialog->accept();

    setConnectionInfo(ip, name);
}

void TimingPoint::noConnectionInfo() {
    // The user cancelled the creation, use a default "Finish" and IP 127.0.0.1
    setConnectionInfo("127.0.0.1", "Finish");
}

void TimingPoint::setConnectionInfo(QString ip, QString name) {
    // Set internal variable
    ipAddressString = ip;

    // Set the IP address label
    setIpAddress();

    // Set the title
    setTitle(name);

    // Make sure we have a folder created for this timing point
    subDirectory = mainFolder + name + "/";
    QDir *dir = new QDir(subDirectory);
    if(!dir->exists())
        dir->mkpath(subDirectory);

    // Save the IP to a file
    QFile settingsFile(subDirectory + ".settings");
    settingsFile.open(QIODevice::WriteOnly);
    QTextStream out(&settingsFile);
    out << ip;
    settingsFile.flush();
    settingsFile.close();

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

void TimingPoint::startBackgroundThread(QString ip, QString subDir) {
    // Start the separate thread and move the socket to it
    QThread *networkThread = new QThread();
    MyTcpSocket *socket = new MyTcpSocket(ip, subDir);

    socket->moveToThread(networkThread);

    // Connect signals and slots
    connect(socket, SIGNAL(connected()), this, SLOT(setConnected()));
    connect(socket, SIGNAL(newImage(QString)), this, SLOT(addNewImage(QString)));
    connect(socket, SIGNAL(finished()), this, SLOT(setDisconnected()));
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
        changeImage(1);
        imageSlider->setSliderPosition(1);
    }
}
