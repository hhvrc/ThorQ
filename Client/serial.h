#ifndef SERIAL_H
#define SERIAL_H

#include <QObject>

class QSerialPort;

class Serial : public QObject
{
	Q_OBJECT
public:
	Serial();

	void SendShock(int strength);
	void SendVibration(int strength);
	void SendBeep(int beeps);
	void SetAuto(int shock, int vibration, int beeps);
private:
	QSerialPort* m_port;
};

#endif // SERIAL_H
