#include <QApplication>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>
#include <QDialogButtonBox>
#include <QObject>

#include "timingpoint.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    TimingPoint tPoint;
    tPoint.show();

    return app.exec();
}
