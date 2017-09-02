#include <QFileDialog>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>

#include "timingpoint.h"

TimingPoint::TimingPoint(QWidget *parent) : QWidget(parent)
{
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
    ipAddressLabel->setText(tr("IP: 0.0.0.0"));

    serverStatus = new QLabel();
    serverStatus->setText(tr("Server Status: Disconnected"));

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

    // Create a file dialog bog
    QFileDialog *fileDialog = new QFileDialog();
    QString mainFolder;
    fileDialog->setFileMode(QFileDialog::Directory);
    mainFolder = fileDialog->exec();
}

