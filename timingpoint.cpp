#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QImage>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
#include <QThread>

#include <QtGui/QSpacerItem>

#include "mytcpsocket.h"
#include "timingpoint.h"
#include "timingpointinfo.h"

TimingPoint::TimingPoint(QString directory, QString name, QString ip, QWidget *parent) : QWidget(parent)
{
    commonSetupCode(directory);

    setConnectionInfo(ip, name);
}

TimingPoint::TimingPoint(QString directory, QWidget *parent) : QWidget(parent)
{
    commonSetupCode(directory);

    // Create a dialog asking for connection info
    dialog = new QDialog(this);
    dialog->setFixedSize(210, 110);
    TimingPointInfo info(dialog);

    // Connect things together
    connect(&info, SIGNAL(setupCompleted(QString, QString)), this, SLOT(setConnectionInfo(QString,QString)));

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

    // Choose our image
    QImage image("/home/jon/Firefox_wallpaper.png");
    imageHolder->setPixmap(QPixmap::fromImage(image));

    // Scroll bar to switch among images
    imageSlider = new QSlider();
    imageSlider->setMinimum(0);
    imageSlider->setMaximum(1);
    imageSlider->setOrientation(Qt::Horizontal);

    // Add buttons
    QPushButton *nextButton = new QPushButton();
    nextButton->setText(tr("Next Person"));
    reconnectButton = new QPushButton();
    reconnectButton->setText(tr("Reconnect"));
    reconnectButton->setEnabled(false);
    changeIpButton = new QPushButton();
    changeIpButton->setText(tr("Change IP"));
    changeIpButton->setEnabled(false);

    // Add line edit for bib number
    QLineEdit *bibNumEdit = new QLineEdit();
    QLabel *bibNumLabel = new QLabel();
    bibNumLabel->setText(tr("Bib Number:"));
    bibNumLabel->setBuddy(bibNumEdit);

    // Add info labels
    timestampLabel = new QLabel(tr("Timstamp:"));
    ipAddressLabel = new QLabel();
    serverStatus = new QLabel("Server Status: Disconnected");

    // A spacer
    QSpacerItem *spacer = new QSpacerItem(40, 60, QSizePolicy::Minimum, QSizePolicy::Expanding);

    // Our layout
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setContentsMargins(5, 5, 5, 5);
    gridLayout->addWidget(bibNumLabel, 1, 1, 1, 1);
    gridLayout->addWidget(ipAddressLabel, 4, 1, 1, 2);
    gridLayout->addItem(spacer, 3, 1, 1, 2);
    gridLayout->addWidget(nextButton, 2, 1, 1, 2);
    gridLayout->addWidget(serverStatus, 5, 1, 1, 2);
    gridLayout->addWidget(bibNumEdit, 1, 2, 1, 1);
    gridLayout->addWidget(imageHolder, 0, 0, 6, 1);
    gridLayout->addWidget(timestampLabel, 0, 1, 1, 2);
    gridLayout->addWidget(reconnectButton, 6, 1, 1, 1);
    gridLayout->addWidget(changeIpButton, 6, 2, 1, 1);
    gridLayout->addWidget(imageSlider, 6, 0, 1, 1);

    setLayout(gridLayout);

    // Button connections
    connect(reconnectButton, SIGNAL(clicked(bool)), this, SLOT(reconnectToServer()));
    connect(changeIpButton, SIGNAL(clicked(bool)), this, SLOT(changeIpDialog()));
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
    }
}

void TimingPoint::reconnectToServer() {
    startBackgroundThread(ipAddressString, subDirectory);
}

void TimingPoint::setConnectionInfo(QString ip, QString name) {
    // Set internal variable
    ipAddressString = ip;

    // Close dialog box
    dialog->accept();

    // Set the IP address label
    setIpAddress();

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
    imagePaths = subDir.entryList(filter, QDir::Files);

    // Set the image slider length to the number of images we have
    imageSlider->setMaximum(imagePaths.length() - 1);

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
    qDebug("got new image");
    // Add the new image to the list of paths
    imagePaths.append(fileName);

    // Increment the slider's maximum
    imageSlider->setMaximum(imageSlider->maximum() + 1);
}
