#ifndef SUMMITEMULATOR_H
#define SUMMITEMULATOR_H

#include <QLabel>
#include <QObject>
#include <QPushButton>
#include <QString>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>


class SummitEmulator : public QObject
{
    Q_OBJECT
public:
    SummitEmulator(QSerialPortInfo info, QObject *parent = 0);
    ~SummitEmulator();

    void sendData(int device, int channel, QString bib, QString time);
private:
    QSerialPort *serialPort;

    int counter;
signals:
private slots:
public slots:
};

#endif // SUMMITEMULATOR_H
