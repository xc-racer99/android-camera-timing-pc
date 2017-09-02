#ifndef TIMINGPOINTINFO_H
#define TIMINGPOINTINFO_H

#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class TimingPointInfo : public QWidget
{
    Q_OBJECT
public:
    explicit TimingPointInfo(QWidget *parent = 0);
private:
    QPushButton *connectButton;
    QLineEdit *hostLineEdit;
    QLineEdit *timingPointLineEdit;
signals:
    void setupCompleted(QString ip, QString name);
private slots:
    void enableConnectButton();
    void onConnectClicked();
};

#endif // TIMINGPOINTINFO_H
