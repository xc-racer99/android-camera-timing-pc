#include <QDir>
#include <QGridLayout>
#include <QInputDialog>
#include <QSlider>
#include <QThread>

#include "mytcpsocket.h"
#include "timingcamera.h"

TimingCamera::TimingCamera(QString dir, QString ip, QObject *parent) : QObject(parent)
{
    directory = dir;

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

    // Setup a QLabel which holds the image
    imageHolder = new QLabel();
    imageHolder->setBackgroundRole(QPalette::Base);
    imageHolder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    imageHolder->setScaledContents(true);

    // Choose our blank image
    imagePaths.append(":/images/images/No_image.png");

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

    // Create a list of images already present
    QStringList filter("*.jpg");
    QFileInfoList initialFileInfo = temp.entryInfoList(filter, QDir::Files, QDir::Name);
    for(int i = 0; i < initialFileInfo.length(); i++)
        imagePaths.append(initialFileInfo.at(i).absoluteFilePath());

    // Start the image saving thread
    startBackgroundThread();
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
    while(index >= imagePaths.length())
        imagePaths.append(":/images/images/No_image.png");
    QImage image(imagePaths.at(index));
    QImage scaledImage = image.scaledToWidth(512);
    imageHolder->setPixmap(QPixmap::fromImage(scaledImage));
    imageHolder->setMaximumSize(scaledImage.width(), scaledImage.height());
}

void TimingCamera::addNewImage(QString fileName) {
    // Add the new image to the list of paths
    imagePaths.append(fileName);

    emit newImage();
}
