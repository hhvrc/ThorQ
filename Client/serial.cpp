#include "serial.h"

#include <QDebug>
#include <QThread>

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

Serial::Serial()
    : m_serial(new QSerialPort(this))
{
    connect(m_serial, &QSerialPort::readyRead, this, &Serial::readData);
}

void Serial::FindCollar()
{
	for (auto &port : QSerialPortInfo::availablePorts())
    {
        if (!port.isNull() && port.vendorIdentifier() == 9025)
        {
            m_serial->setPort(port);
            if (m_serial->open(QIODevice::ReadWrite))
            {
                char c = 9;
                m_serial->write(&c, 1);
                m_serial->waitForBytesWritten(-1);
                m_serial->waitForReadyRead(1000);
                /*char datb[4] { 1, 2, 1, 8 };
                for (int i = 0; i < 10;  i++)
                {
                    m_serial->write(datb, 4);
                    m_serial->waitForBytesWritten(-1);
                    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                }*/
                m_serial->close();
            }
        }
    }
}

QByteArray arr;

void Serial::readData()
{
    arr += m_serial->readAll();

    if (arr.size() > 5)
        qDebug() << arr.startsWith("ThorQ");
}
