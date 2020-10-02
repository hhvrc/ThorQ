#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QSet>

#include "typedefs_global.h"

namespace ThorQ {
class Server : public QObject
{
    Q_OBJECT
public:
    static QString Version();
    static bool Initialize();
    static void DeInitialize();

    Server(QObject* parent = nullptr);
    ~Server();

    bool setup(std::uint16_t port, std::size_t maxPeers, std::uint8_t channelCount, bool noDelay);

    bool start();
    void stop();

    void cleanup();

    std::uint16_t HeartbeatInterval();
    void SetHeartbeatInterval(std::uint16_t msInterval);

    std::uint64_t totalDataSent();
    std::uint64_t totalPacketsSent();
    std::uint64_t totalDataReceived();
    std::uint64_t totalPacketsReceived();
private:
    void handleEventConnection(const ENetEvent& event);
    void handleEventMessage(const ENetEvent &event);
    void handleEventDisconnect(const ENetEvent& event);
    void handleEventTimeout(const ENetEvent& event);

    ENetHost* m_host;

    std::atomic<std::uint16_t> m_heartbeatInterval;

    std::atomic<std::uint64_t> m_totalSentData;
    std::atomic<std::uint64_t> m_totalSentPackets;
    std::atomic<std::uint64_t> m_totalReceivedData;
    std::atomic<std::uint64_t> m_totalReceivedPackets;
};
}

#endif // SERVER_H
