#ifndef TIMINGPOINT_H
#define TIMINGPOINT_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QDialog>
#include <QLineEdit>
#include <QFile>

class TimingPoint : public QWidget
{
    Q_OBJECT
public:
    explicit TimingPoint(QString directory, QWidget *parent = 0);
    explicit TimingPoint(QString directory, QString name, QString ip, QWidget *parent = 0);
private:
    void commonSetupCode(QString directory);
    void setConnectionInfo(QString ip, QString name);

    QLabel *imageHolder;
    QSlider *imageSlider;
    QLabel *serverStatus;
    QLabel *ipAddressLabel;
    QString ipAddressString;
    QDialog *dialog;
    QString mainFolder;
    QString subDirectory;
    QPushButton *reconnectButton;
    QPushButton *changeIpButton;
    QPushButton *nextButton;
    QLabel *timestampLabel;
    QLabel *actualTimestamp;
    QStringList imagePaths;
    QLineEdit *bibNumEdit;
    QFile *csvFile;
signals:
private slots:
    void gotConnectionInfo(QString ip, QString name);

    // Status handlers
    void setIpAddress();
    void setConnected();
    void setDisconnected();

    // Button handlers
    void reconnectToServer();
    void changeIpDialog();
    void submitButtonPushed();

    // Image-related handlers
    void startBackgroundThread(QString ip, QString name);
    void addNewImage(QString fileName);
    void changeImage(int index);
public slots:
};

#endif // TIMINGPOINT_H
