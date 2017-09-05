#ifndef TIMINGPOINT_H
#define TIMINGPOINT_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QDialog>

class TimingPoint : public QWidget
{
    Q_OBJECT
public:
    explicit TimingPoint(QString directory, QWidget *parent = 0);
    explicit TimingPoint(QString directory, QString name, QString ip, QWidget *parent = 0);
private:
    void commonSetupCode(QString directory);

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
    QLabel *timestampLabel;
    QStringList imagePaths;
signals:
private slots:
    void setConnectionInfo(QString ip, QString name);

    // Status handlers
    void setIpAddress();
    void setConnected();
    void setDisconnected();
    void setTimestamp(QString time);

    // Button handlers
    void reconnectToServer();
    void changeIpDialog();

    // Image-related handlers
    void startBackgroundThread(QString ip, QString name);
    void addNewImage(QString fileName);
    void changeImage(int index);
public slots:
};

#endif // TIMINGPOINT_H
