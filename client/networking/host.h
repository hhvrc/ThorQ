#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTimer>

#include <enet.h>

#include "message.h"

namespace ThorQ {
namespace Networking {
class Message;
class ConnectionHandler;
class Host : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Host)

    Host(ENetHost* host, QObject* parent);
public:
    static const char* Version();
    static bool Initialize();
    static void DeInitialize();

    static ThorQ::Networking::Host* CreateHost(std::uint8_t channelLimit, QObject* parent);
    static ThorQ::Networking::Host* CreateHost(QString hostname, std::uint16_t port, std::size_t peerLimit, std::uint8_t channelLimit, QObject* parent);
    ~Host();

    std::uint64_t txData() const;
    std::uint32_t txSpeed() const;
    std::uint64_t rxData() const;
    std::uint32_t rxSpeed() const;
public slots:
    void connect(QString hostname, std::uint16_t port, std::uint8_t channelCount, ThorQ::Networking::ConnectionHandler* handler);
signals:
    void connectionIncoming(ThorQ::Networking::ConnectionHandler* handler);

    void txDataChanged(std::uint64_t txData);
    void txSpeedChanged(std::uint32_t txSpeed);
    void rxDataChanged(std::uint64_t rxData);
    void rxSpeedChanged(std::uint32_t rxSpeed);

    void warning(const QString& what);
    void error(const QString& what);
private slots:
    void service();
    void updateStats();

    friend ThorQ::Networking::ConnectionHandler;

    void sendUdp(ENetPeer* peer, ThorQ::Networking::Message message);

    void disconnect(ENetPeer* peer, std::uint32_t reason);
    void disconnectNow(ENetPeer* peer, std::uint32_t reason);
    void disconnectLater(ENetPeer* peer, std::uint32_t reason);
private:
    QTimer m_serviceTimer;
    QTimer m_statisticsTimer;

    ENetHost* m_host;

    std::atomic_uint64_t m_totalDataTx;
    std::atomic_uint64_t m_totalDataRx;
    std::atomic_uint32_t m_speedTx;
    std::atomic_uint32_t m_speedRx;
};
}
}

#endif // CLIENT_H
