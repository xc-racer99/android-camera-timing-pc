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
    SummitEmulator(QObject *parent = 0);
    ~SummitEmulator();

    int getDeviceNumber();

    void setDeviceNumber(int number);
    void setPort(QSerialPortInfo info);

    int deviceNumber;

    void initialize();
private:
    QSerialPort *serialPort;

    int counter;
signals:
private slots:
    void sendAck();
public slots:
    void sendData(int channel, QString bib, QString time);
};

#endif // SUMMITEMULATOR_H
