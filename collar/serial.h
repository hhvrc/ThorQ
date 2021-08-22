#ifndef SERIAL_H
#define SERIAL_H

#include <QObject>

class QSerialPort;

class CollarSerial : public QObject
{
	Q_OBJECT
public:
    CollarSerial(QObject* parent = nullptr);

signals:
    void FoundCollar();
public slots:
    void FindCollar();
    void SendShock(unsigned int strength);
    void SendVibration(unsigned int strength);
    void SendBeep(unsigned int beeps);
    void SetAuto(unsigned int shock, unsigned int vibration, unsigned int beeps);
private slots:
    /**
     * @brief readData
     */
    void readData();
private:
    QSerialPort* m_serial;
};

#endif // SERIAL_H
