#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
#include <QThread>

#include "mytcpsocket.h"
#include "timingpoint.h"
#include "timingpointinfo.h"

TimingPoint::TimingPoint(QString directory, QWidget *parent) : QWidget(parent)
{
    // Pass the directory along
    mainFolder = &directory;

    // Create our strings
    ipAddressString = new QString("0.0.0.0");
    serverStatusString = new QString("Disconnected");

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

    // Add line edit for bib number
    QLineEdit *bibNumEdit = new QLineEdit();
    QLabel *bibNumLabel = new QLabel();
    bibNumLabel->setText(tr("Bib Number:"));
    bibNumLabel->setBuddy(bibNumEdit);

    // Add info labels
    ipAddressLabel = new QLabel();

    serverStatus = new QLabel("Server Status: Disconnected");

    // Our layout
    QGridLayout *layout = new QGridLayout();
    layout->addWidget(imageHolder, 0, 0, 4, 6);
    layout->addWidget(bibNumLabel, 0, 7, 1, 1);
    layout->addWidget(bibNumEdit, 0, 8, 1, 1);
    layout->addWidget(nextButton, 1, 7, 1, 2);
    layout->addWidget(imageSlider, 5, 0, 1, 6);
    layout->addWidget(ipAddressLabel, 4, 7, 1, 2);
    layout->addWidget(serverStatus, 5, 7, 1, 2);

    setLayout(layout);

    setWindowTitle("Timing Point");

    // Create a dialog asking for connection info
    dialog = new QDialog(this);
    dialog->setFixedSize(210, 110);
    TimingPointInfo info(dialog);

    // Connect things together
    connect(&info, SIGNAL(setupCompleted(QString, QString)), this, SLOT(setConnectionInfo(QString,QString)));

    // Show the dialog
    dialog->exec();
}

void TimingPoint::setIpAddress() {
    ipAddressLabel->setText(tr("IP: %1").arg(*ipAddressString));
}

void TimingPoint::setConnected() {
    serverStatus->setText("Server Status: Connected");
}

void TimingPoint::setDisconnected() {
    serverStatus->setText("Server Status: Disconnected");
}

void TimingPoint::setConnectionInfo(QString ip, QString name) {
    // Set internal variable
    ipAddressString = &ip;

    // Close dialog box
    dialog->accept();

    // Set the IP address label
    setIpAddress();

    // Make sure we have a folder created for this timing point
    QString subDir = *mainFolder + name + "/";
    QDir *dir = new QDir(subDir);
    if(!dir->exists())
        dir->mkpath(subDir);

    // Save the IP to a file
    QFile settingsFile(subDir + ".settings");
    settingsFile.open(QIODevice::WriteOnly);
    QTextStream out(&settingsFile);
    out << ip;
    settingsFile.flush();
    settingsFile.close();

    // Start the image saving thread
    startBackgroundThread(ip, subDir);
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
}
