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
private:
    QLabel *imageHolder;
    QSlider *imageSlider;
    QLabel *serverStatus;
    QLabel *ipAddressLabel;
    QString *ipAddressString;
    QString *serverStatusString;
    QDialog *dialog;
    QString *mainFolder;
signals:
private slots:
    void setConnectionInfo(QString ip, QString name);
    void setIpAddress();
    void setConnectionStatus();
public slots:
};

#endif // TIMINGPOINT_H
