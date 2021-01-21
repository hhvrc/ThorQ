#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include <cstdint>
#include <atomic>

#include <QObject>

#include <enet.h>

#include "message.h"

namespace ThorQ {
namespace Networking {
class Host;
class ConnectionHandler final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ConnectionHandler)
public:
    ConnectionHandler(QObject* parent);

    std::uint32_t rtt() const;
    std::uint64_t txData() const;
    std::uint64_t rxData() const;
    std::uint32_t txSpeed() const;
    std::uint32_t rxSpeed() const;
signals:
    void rttChanged(std::uint16_t rtt);
    void txDataChanged(std::uint64_t txData);
    void txSpeedChanged(std::uint32_t txSpeed);
    void rxDataChanged(std::uint64_t rxData);
    void rxSpeedChanged(std::uint32_t rxSpeed);

    void udpReceived(ThorQ::Networking::Message message);

    void connected(std::uint32_t reason);
    void disconnected(std::uint32_t reason);
public slots:
    void sendUdp(ThorQ::Networking::Message message);

    void disconnect(std::uint32_t reason);
    void disconnectNow(std::uint32_t reason);
    void disconnectLater(std::uint32_t reason);
private slots:
    friend ThorQ::Networking::Host;

    void clear();
    void setHost(ThorQ::Networking::Host* host);
    void setPeer(ENetPeer* peer);
    void updateStats();
    void handleEvent(ENetEvent event);
private:
    ENetPeer* m_peer;
    ThorQ::Networking::Host* m_host;

    std::atomic_uint16_t m_rtt;
    std::atomic_uint64_t m_dataTx;
    std::atomic_uint64_t m_dataRx;
    std::atomic_uint32_t m_speedTx;
    std::atomic_uint32_t m_speedRx;
};
}
}

#endif // CONNECTIONHANDLER_H
