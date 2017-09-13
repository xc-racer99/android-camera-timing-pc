/**
 * Copyright (C) 2017  Jonathan Bakker
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 **/

#include <QByteArray>
#include <QGridLayout>
#include <QThread>
#include <QTimer>

#include "summitemulator.h"
#include "3rd-Party/libcrc/include/checksum.h"

SummitEmulator::SummitEmulator(QSerialPortInfo info, QString deviceNum, QObject *parent) : QObject(parent)
{
    // Initialize the QSerialPort
    serialPort = new QSerialPort();
    serialPort->setPort(info);

    // Convert the device number
    bool ok;
    int temp = deviceNum.toInt(&ok);
    if(ok)
        deviceNumber = temp;
    else
        deviceNumber = 1;

    counter = 1;
}

SummitEmulator::~SummitEmulator() {
    serialPort->close();
}

void SummitEmulator::initialize() {
    if(!serialPort->portName().isEmpty()) {
        serialPort->setBaudRate(QSerialPort::Baud9600);
        serialPort->setDataBits(QSerialPort::Data8);
        serialPort->setStopBits(QSerialPort::OneStop);
        serialPort->setFlowControl(QSerialPort::NoFlowControl);
        if(serialPort->open(QIODevice::ReadWrite)) {
            qDebug("Succesfully opened serial port");

            // HACK - Start sending that we're connected every 3 seconds
            QTimer *timer = new QTimer(this);
            timer->setInterval(3000);
            connect(timer, SIGNAL(timeout()), this, SLOT(sendAck()));
            timer->start();
        }
    }
}

void SummitEmulator::sendData(int channel, QString bib, QString time) {
    if(serialPort->isOpen()) {
        QString data = QString("%1\u0009%2\u00090\u00090\u0009%3\u0009b\u0009%4\u0009 %5").arg(deviceNumber).arg(counter).arg(channel).arg(bib).arg(time);
        QByteArray temp = data.toLatin1();
        quint16 crc = crc_16((unsigned const char*)strdup(temp.constData()), (size_t)temp.length());
        QByteArray toSend(QString("{%1}%2\u000D\u000A").arg(data).arg(crc, 0, 16).toLatin1());
        qDebug("Sending data %s", toSend.constData());
        serialPort->write(toSend);
        serialPort->flush();
    }
    counter++;
}

void SummitEmulator::sendAck() {
    QString dataToWrite = QString("AK %1").arg(deviceNumber);
    QByteArray tempData = dataToWrite.toLatin1();
    quint16 crc = crc_16((unsigned const char*)strdup(tempData.constData()), (size_t)tempData.length());
    QByteArray toWrite(QString("{%1}%2\u000D\u000A").arg(dataToWrite).arg(crc, 0, 16).toLatin1());
    serialPort->write(toWrite);
}
