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

#include "summitemulator.h"

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
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(receiveData()));
    serialPort->setBaudRate(QSerialPort::Baud9600);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);
    if(serialPort->open(QIODevice::ReadWrite))
        qDebug("Succesfully opened serial port");
}

void SummitEmulator::sendData(int channel, QString bib, QString time) {
    if(serialPort->isOpen()) {
        QString data = QString("%1\t%2\t0\t0\t%3\tb\t\t%4\t%5").arg(deviceNumber).arg(counter).arg(channel).arg(bib).arg(time);
        QByteArray temp = data.toLatin1();
        quint16 crc = qChecksum(temp.data(), temp.length());
        QByteArray toSend(QString("{%1}%2X\n").arg(data).arg(crc).toLatin1());
        serialPort->write(toSend);
        serialPort->flush();
    }
    counter++;
}

void SummitEmulator::receiveData() {
    QString input = serialPort->readLine();
    // Remove leading {
    input.resize(1);
    QStringList parts = input.split('}');
    qDebug("Data is %s", input.toLatin1().constData());
    if(parts.length() == 2) {
        // This is the actual data
        QString data = parts.at(0);
        // This is the CRC-16
        QString crc = parts.at(1);
        QByteArray temp = data.toLatin1();
        if(qChecksum(temp.data(), temp.length()) == crc.toInt(0, 16)) {
            // This is an actual packet, determine what to do
            if(data.contains(".+AK.+") || data.contains(".+SY.+")){
                // This is an AK from somewhere else, or a sync command, both of which we ignore
            } else if(data.contains(QString("TK %1").arg(deviceNumber))) {
                QString dataToWrite = QString("AK %1").arg(deviceNumber);
                QByteArray tempData = dataToWrite.toLatin1();
                quint16 crc = qChecksum(tempData.data(), tempData.length());
                QByteArray toWrite(QString("{%1}%2X\n").arg(dataToWrite).arg(crc).toLatin1());
                serialPort->write(toWrite);
            } else {
                // Unknown command
                qDebug("Unknown command %s", data.toLatin1().constData());
            }
        }
    }
}

