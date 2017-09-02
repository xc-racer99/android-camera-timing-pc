#ifndef TIMINGPOINT_H
#define TIMINGPOINT_H

#include <QWidget>
#include <QSlider>
#include <QLabel>

class TimingPoint : public QWidget
{
    Q_OBJECT
public:
    explicit TimingPoint(QWidget *parent = 0);
private:
    QLabel *imageHolder;
    QSlider *imageSlider;
    QLabel *serverStatus;
    QLabel *ipAddressLabel;

signals:

public slots:
};

#endif // TIMINGPOINT_H
