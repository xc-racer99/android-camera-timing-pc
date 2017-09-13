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
    SummitEmulator(QSerialPortInfo info, QString deviceNum, QObject *parent = 0);
    ~SummitEmulator();

    int deviceNumber;

    void initialize();
private:
    QSerialPort *serialPort;

    int counter;
signals:
private slots:
    void receiveData();
public slots:
    void sendData(int channel, QString bib, QString time);
};

#endif // SUMMITEMULATOR_H
