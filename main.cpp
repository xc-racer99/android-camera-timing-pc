#include <QApplication>

#include "mainwindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    app.setWindowIcon(QIcon(":/images/images/icon.png"));

    MainWindow window;

    window.showMaximized();

    return app.exec();
}
