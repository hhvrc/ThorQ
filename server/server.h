#ifndef SERVER_H
#define SERVER_H

#include <queue>
#include <unordered_set>

#include <flatbuffers/flatbuffers.h>

#include "enums.h"
#include "typedefs_global.h"
#include "typedefs_server.h"

#include "concurrentqueue.h"

namespace ThorQ {
class Server
{
public:
    static const char* Version();
    static bool Initialize();
    static void DeInitialize();

    Server();
    ~Server();

    enum class ServerStatus
    {
        Error,
        Stopped,
        Starting,
        Running,
        Stopping
    };

    ServerStatus status() const;

    bool start(std::uint16_t port, std::size_t maxPeers, std::uint8_t channelCount, bool noDelay);
    void stop();

    std::uint32_t HeartbeatInterval() const;
    void SetHeartbeatInterval(std::uint32_t msInterval);

    std::uint64_t totalDataSent() const;
    std::uint64_t totalPacketsSent() const;
    std::uint64_t totalDataReceived() const;
    std::uint64_t totalPacketsReceived() const;

    void broadcastAnnouncement(const flatbuffers::DetachedBuffer& packet, bool reliable, bool unsequenced);
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
    bool tryQueueMessage(const QueuedMessage& message);
private:
    void run();

    void handleEventConnection(const ENetEvent& event);
    void handleEventMessage(const ENetEvent &event);
    void handleEventDisconnect(const ENetEvent& event);
    void handleEventTimeout(const ENetEvent& event);

    ENetHost* m_host;

    std::thread* m_thread;
    std::atomic_bool m_run;

    std::atomic<ServerStatus> m_status;

    std::atomic_uint32_t m_heartbeatInterval;

    std::atomic_uint64_t m_totalSentData;
    std::atomic_uint64_t m_totalSentPackets;
    std::atomic_uint64_t m_totalReceivedData;
    std::atomic_uint64_t m_totalReceivedPackets;

    moodycamel::ConcurrentQueue<QueuedMessage> m_txQueue;
    moodycamel::ProducerToken m_txToken;

    moodycamel::ConcurrentQueue<QueuedMessage> m_rxQueue;
    moodycamel::ConsumerToken m_rxToken;

    moodycamel::ConcurrentQueue<ENetPacket*> m_broadcastQueue;
    moodycamel::ConsumerToken m_broadcastToken;
};
}

#endif // SERVER_H
