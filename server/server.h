#ifndef SERVER_H
#define SERVER_H

#include <queue>
#include <unordered_set>

#include <QObject>
#include <QRunnable>
#include <QReadWriteLock>

#include "enums.h"
#include "typedefs_global.h"
#include "typedefs_server.h"

#include "concurrentqueue.h"

namespace ThorQ {
class Server : public QObject, private QRunnable
{
    Q_OBJECT
public:
    static QString Version();
    static bool Initialize();
    static void DeInitialize();

    Server(QObject* parent = nullptr);
    ~Server();

    bool setup(std::uint16_t port, std::size_t maxPeers, std::uint8_t channelCount, bool noDelay);

    void start();
    void stop();

    enum class ServerStatus
    {
        Stopped,
        Stopping,
        Starting,
        Started,
    };
    ServerStatus status();

    void cleanup();

    std::uint32_t HeartbeatInterval();
    void SetHeartbeatInterval(std::uint32_t msInterval);

    std::uint64_t totalDataSent();
    std::uint64_t totalPacketsSent();
    std::uint64_t totalDataReceived();
    std::uint64_t totalPacketsReceived();

    void broadcastAnnouncement(const ThorQ::THORQ_PAYLOAD& payload, bool reliable, bool unsequenced);
signals:
    void statusChanged(const ServerStatus& status);
    void heartbeatChanged(const std::uint32_t& status);
protected:
    friend Instance;
    friend MessageDispatcher;

    struct QueuedMessage
    {
        ENetPeer* peer;
        ENetPacket* packet;
        std::uint8_t channel; /// THORQ_CHANNEL
    };

    bool tryGetMessage(QueuedMessage& message);
    void queueMessage(const QueuedMessage& message);
private:
    void setStatus(ServerStatus status);

    void run() override;

    void handleEventConnection(const ENetEvent& event);
    void handleEventMessage(const ENetEvent &event);
    void handleEventDisconnect(const ENetEvent& event);
    void handleEventTimeout(const ENetEvent& event);

    ENetHost* m_host;

    std::atomic<bool> m_shouldRun;
    std::atomic<ServerStatus> m_status;

    std::atomic<std::uint32_t> m_heartbeatInterval;

    std::atomic<std::uint64_t> m_totalSentData;
    std::atomic<std::uint64_t> m_totalSentPackets;
    std::atomic<std::uint64_t> m_totalReceivedData;
    std::atomic<std::uint64_t> m_totalReceivedPackets;

    moodycamel::ConcurrentQueue<QueuedMessage> m_receivedMessages;

    moodycamel::ConcurrentQueue<QueuedMessage> m_queuedMessages;
    moodycamel::ConcurrentQueue<ENetPacket*> m_queuedBroadcasts;
};
}

#endif // SERVER_H
