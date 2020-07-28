#ifndef SERIAL_H
#define SERIAL_H

#include <QObject>

class QSerialPort;

class Serial : public QObject
{
	Q_OBJECT
public:
	Serial();

    void FindCollar();

    void SendShock(unsigned int strength);
    void SendVibration(unsigned int strength);
    void SendBeep(unsigned int beeps);
    void SetAuto(unsigned int shock, unsigned int vibration, unsigned int beeps);
private slots:
    void readData();
private:
    QSerialPort* m_serial;
};

#endif // SERIAL_H
