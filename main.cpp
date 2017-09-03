#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>

#include "timingpoint.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Create a file dialog box
    QString directory;
    while(directory.isEmpty()) {
        QFileDialog *fileDialog = new QFileDialog();
        fileDialog->setFileMode(QFileDialog::Directory);
        if(fileDialog->exec()) {
            directory = fileDialog->selectedFiles().at(0) + QDir::separator();
        }
    }

    TimingPoint tPoint(directory);
    tPoint.show();

    return app.exec();
}
