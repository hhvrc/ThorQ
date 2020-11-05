#ifndef SERVER_H
#define SERVER_H

#include <span>
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

    void broadcastAnnouncement(const std::span<std::uint8_t> payload, bool reliable, bool unsequenced);
protected:
    friend Instance;
    friend MessageDispatcher;

    struct QueuedMessage
    {
        ENetPeer* peer;
        ENetPacket* packet;
        std::uint8_t channel;
    };
    struct QueuedDisconnect
    {
        ENetPeer* peer;
        THORQ_DISCONNECT_REASON reason;
        bool force;
    };

    bool tryGetMessage(moodycamel::ConsumerToken token, QueuedMessage& message);
    bool tryQueueMessage(moodycamel::ProducerToken token, const QueuedMessage& message);
    bool disconnectPeer(moodycamel::ProducerToken token, QueuedDisconnect& disconnect);
private:
    void run();

    void handleEventConnection(const ENetEvent& event);
    void handleEventMessage(const ENetEvent &event);
    void handleEventDisconnect(const ENetEvent& event);
    void handleEventTimeout(const ENetEvent& event);

    ENetHost* m_host;

    std::thread* m_thread;
    std::atomic_bool m_run;

    std::vector<ThorQ::MessageDispatcher*> m_dispatchers;

    std::atomic<ServerStatus> m_status;

    std::atomic_uint32_t m_heartbeatInterval;

    std::atomic_uint64_t m_totalSentData;
    std::atomic_uint64_t m_totalSentPackets;
    std::atomic_uint64_t m_totalReceivedData;
    std::atomic_uint64_t m_totalReceivedPackets;

    moodycamel::ConsumerToken m_rxToken;
    moodycamel::ConcurrentQueue<QueuedMessage> m_rxQueue;

    moodycamel::ProducerToken m_txToken;
    moodycamel::ConcurrentQueue<QueuedMessage> m_txQueue;

    moodycamel::ConsumerToken m_broadcastToken;
    moodycamel::ConcurrentQueue<ENetPacket*> m_broadcastQueue;

    moodycamel::ConsumerToken m_disconnectToken;
    moodycamel::ConcurrentQueue<QueuedDisconnect> m_disconnectQueue;
};
}

#endif // SERVER_H
