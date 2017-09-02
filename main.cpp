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

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    TimingPointInfo info;
    info.show();

    return app.exec();
}
