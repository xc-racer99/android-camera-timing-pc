#include <QApplication>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>
#include <QDialogButtonBox>
#include <QObject>
#include "timingpointinfo.h"

TimingPointInfo::TimingPointInfo(QWidget *parent) : QWidget(parent)
{
    // Show a dialog asking for the IP address and timing point name

    QLabel *hostLabel = new QLabel(tr("&Server IP:"));
    QLabel *timingPointLabel = new QLabel(tr("&Point Name:"));

    hostLineEdit = new QLineEdit;
    timingPointLineEdit = new QLineEdit;

    hostLabel->setBuddy(hostLineEdit);
    timingPointLabel->setBuddy(timingPointLineEdit);

    connectButton = new QPushButton(tr("Connect"));
    connectButton->setDefault(true);
    connectButton->setEnabled(false);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    buttonBox->addButton(connectButton, QDialogButtonBox::ActionRole);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(hostLabel, 0, 0);
    mainLayout->addWidget(hostLineEdit, 0, 1);
    mainLayout->addWidget(timingPointLabel, 1, 0);
    mainLayout->addWidget(timingPointLineEdit, 1, 1);
    mainLayout->addWidget(buttonBox, 2, 0, 1, 2);

    connect(hostLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enableConnectButton()));
    connect(timingPointLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enableConnectButton()));
    connect(connectButton, SIGNAL(clicked()),
            this, SLOT(onConnectClicked()));

    setLayout(mainLayout);

    setWindowTitle(tr("Connect to Timing Point"));
}

void TimingPointInfo::enableConnectButton() {
    if (!hostLineEdit->text().isEmpty() && !timingPointLineEdit->text().isEmpty())
        connectButton->setEnabled(true);
}

void TimingPointInfo::onConnectClicked() {
    emit setupCompleted(hostLineEdit->text(), timingPointLineEdit->text());
}
