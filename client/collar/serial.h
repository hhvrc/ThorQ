#ifndef SERIAL_H
#define SERIAL_H

#include <QObject>

class QSerialPort;

/**
 * @brief The Serial class
 */
class Serial : public QObject
{
	Q_OBJECT
public:
    /**
     * @brief Serial
     */
	Serial();

    /**
     * @brief FindCollar
     */
    void FindCollar();

    /**
     * @brief SendShock
     * @param strength
     */
    void SendShock(unsigned int strength);

    /**
     * @brief SendVibration
     * @param strength
     */
    void SendVibration(unsigned int strength);

    /**
     * @brief SendBeep
     * @param beeps
     */
    void SendBeep(unsigned int beeps);

    /**
     * @brief SetAuto
     * @param shock
     * @param vibration
     * @param beeps
     */
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
