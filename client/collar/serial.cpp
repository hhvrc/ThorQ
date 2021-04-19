#include "serial.h"

#include <QDebug>
#include <QTimer>
#include <QThread>

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

CollarSerial::CollarSerial(QObject* parent)
    : QObject(parent)
    , m_serial(nullptr)
{
}

void CollarSerial::FindCollar()
{
    for (auto& port : QSerialPortInfo::availablePorts())
    {
        QSerialPort* ser = new QSerialPort(port, this);

        if (ser->setBaudRate(QSerialPort::Baud115200)       &&
            ser->setDataBits(QSerialPort::Data8)            &&
            ser->setParity(QSerialPort::NoParity)           &&
            ser->setStopBits(QSerialPort::OneStop)          &&
            ser->setFlowControl(QSerialPort::NoFlowControl)) {

            if (!ser->open(QIODevice::ReadWrite)) {
                qWarning() << "Failed to open" << ser->portName();
                ser->deleteLater();
                continue;
            }

            QTimer* t_kill = new QTimer(ser);
            QTimer* t_retry = new QTimer(ser);

            connect(t_kill, &QTimer::timeout, t_retry, &QTimer::stop);
            connect(t_kill, &QTimer::timeout, ser, &QSerialPort::deleteLater);

            connect(t_retry, &QTimer::timeout, [ser](){
                ser->write("V", 1);
            });

            connect(ser, &QSerialPort::readyRead, [=](){
                if (ser->bytesAvailable() >= 8) {
                    if (ser->readAll().startsWith("ThorQ-")) {
                        m_serial = ser;

                        disconnect(t_kill, &QTimer::timeout, t_retry, &QTimer::stop);
                        disconnect(t_kill, &QTimer::timeout, ser, &QSerialPort::close);
                        disconnect(t_kill, &QTimer::timeout, t_retry, &QObject::deleteLater);

                        t_retry->stop();
                        t_kill->deleteLater();
                        t_retry->deleteLater();

                        connect(ser, &QSerialPort::readyRead, this, &CollarSerial::readData);

                        emit FoundCollar();
                        return;
                    }
                }
            });

            t_kill->setSingleShot(true);
            t_kill->setInterval(10000);

            t_retry->setSingleShot(false);
            t_retry->setInterval(100);

            t_kill->start();
            t_retry->start();
        }
    }
}

void CollarSerial::SendShock(unsigned int strength)
{
    std::string_view str;

    using namespace std::literals;
    switch (strength) {
    case 1:
        str = ">011<"sv;
        break;
    case 2:
        str = ">012<"sv;
        break;
    case 3:
        str = ">013<"sv;
        break;
    case 4:
        str = ">014<"sv;
        break;
    case 5:
        str = ">015<"sv;
        break;
    case 6:
        str = ">016<"sv;
        break;
    case 7:
        str = ">017<"sv;
        break;
    default:
        str = ">010<"sv;
        break;
    }

    if (m_serial != nullptr) {
        m_serial->write(str.data(), str.size());
        m_serial->flush();
    }
}

void CollarSerial::SendVibration(unsigned int strength)
{
    std::string_view str;

    using namespace std::literals;
    switch (strength) {
    case 1:
        str = ">111<"sv;
        break;
    case 2:
        str = ">112<"sv;
        break;
    case 3:
        str = ">113<"sv;
        break;
    case 4:
        str = ">114<"sv;
        break;
    case 5:
        str = ">115<"sv;
        break;
    case 6:
        str = ">116<"sv;
        break;
    case 7:
        str = ">117<"sv;
        break;
    default:
        str = ">110<"sv;
        break;
    }

    if (m_serial != nullptr) {
        m_serial->write(str.data(), str.size());
        m_serial->flush();
    }
}

void CollarSerial::SendBeep(unsigned int beeps)
{
    std::string_view str;

    using namespace std::literals;
    switch (beeps) {
    case 1:
        str = ">211<"sv;
        break;
    case 2:
        str = ">212<"sv;
        break;
    case 3:
        str = ">213<"sv;
        break;
    case 4:
        str = ">214<"sv;
        break;
    case 5:
        str = ">215<"sv;
        break;
    case 6:
        str = ">216<"sv;
        break;
    case 7:
        str = ">217<"sv;
        break;
    default:
        str = ">210<"sv;
        break;
    }

    if (m_serial != nullptr) {
        m_serial->write(str.data(), str.size());
        m_serial->flush();
    }
}

void CollarSerial::SetAuto(unsigned int shock, unsigned int vibration, unsigned int beeps)
{

}

void CollarSerial::readData()
{
    m_serial->clear();
}
