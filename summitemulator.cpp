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

#include "summitemulator.h"

SummitEmulator::SummitEmulator(QSerialPortInfo info, QObject *parent) : QObject(parent)
{
    // Initialize the QSerialPort
    serialPort = new QSerialPort();
    serialPort->setPort(info);
    serialPort->setBaudRate(QSerialPort::Baud9600);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);
    serialPort->open(QIODevice::WriteOnly);

    counter = 1;
}

SummitEmulator::~SummitEmulator() {
    serialPort->close();
}

void SummitEmulator::sendData(int device, int channel, QString bib, QString time) {
    if(serialPort->isOpen()) {
        QString data = QString("%1\t%2\t0\t0\t%3\tb\t\t%4\t%5").arg(device).arg(counter).arg(channel).arg(bib).arg(time);
        QByteArray temp = data.toLatin1();
        quint16 crc = qChecksum(temp.data(), temp.length());
        QByteArray toSend(QString("{%1}%2X").arg(data).arg(crc).toLatin1());
        serialPort->write(toSend);
        serialPort->flush();
    }
    counter++;
}

