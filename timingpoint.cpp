#include <QFileDialog>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>

#include "imagethread.h"
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

void TimingPoint::setConnectionStatus() {
    serverStatus->setText(tr("Server Status: %1").arg(*serverStatusString));
}

void TimingPoint::setConnectionInfo(QString ip, QString name) {
    ipAddressString = &ip;
    dialog->accept();
    setIpAddress();

    ImageThread *backgroundThread = new ImageThread(ip, *mainFolder + name + "/");
    backgroundThread->run();
}
