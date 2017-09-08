#ifndef TIMINGPOINT_H
#define TIMINGPOINT_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QDialog>
#include <QLineEdit>
#include <QFile>
#include <QGroupBox>

class TimingPoint : public QGroupBox
{
    Q_OBJECT
public:
    explicit TimingPoint(QString directory, QString name, QString ip, QWidget *parent = 0);
private:
    void saveSettings();
    void startBackgroundThread(QString ip, QString name);

    // Common properties
    QFile *csvFile;
    QLabel *timestamp;
    QLineEdit *bibNumEdit;
    QPushButton *nextButton;
    QSlider *imageSlider;
    QString subDirectory;

    // Main phone variables
    QGroupBox *statusBox;
    QLabel *imageHolder;
    QLabel *ipAddress;
    QLabel *serverStatus;
    QPushButton *reconnectButton;
    QPushButton *changeIpButton;
    QStringList imagePaths;

signals:
private slots:
    // Status handlers
    void setIpAddress(QString newIp);
    void setConnectionStatus(QString status);

    // Button handlers
    void reconnectToServer();
    void changeIpDialog();
    void submitButtonPushed();
    void plusButtonPushed();
    void minusButtonPushed();

    // Image-related handlers
    void addNewImage(QString fileName);
    void changeImage(int index);
public slots:
};

#endif // TIMINGPOINT_H
