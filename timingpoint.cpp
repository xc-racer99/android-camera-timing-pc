#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>

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

    // Add buttons
    QPushButton *nextButton = new QPushButton();
    nextButton->setText(tr("Next Person"));

    // Add line edit for bib number
    QLineEdit *bibNumEdit = new QLineEdit();
    QLabel *bibNumLabel = new QLabel();
    bibNumLabel->setText(tr("Bib Number:"));
    bibNumLabel->setBuddy(bibNumEdit);

    // Our layout
    QGridLayout *layout = new QGridLayout();
    layout->addWidget(imageHolder, 0, 0, 4, 6);
    layout->addWidget(bibNumLabel, 0, 7, 1, 1);
    layout->addWidget(bibNumEdit, 0, 8, 1, 1);
    layout->addWidget(nextButton, 1, 7, 1, 2);

    setLayout(layout);

    setWindowTitle("Timing Point");
}

