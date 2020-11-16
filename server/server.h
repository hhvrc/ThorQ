#ifndef SERVER_H
#define SERVER_H

#include <span>
#include <queue>
#include <unordered_set>

#include <enet.h>
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

    std::uint32_t heartbeatInterval() const;
    void setHeartbeatInterval(std::uint32_t msInterval);

    std::uint64_t totalDataSent() const;
    std::uint64_t totalPacketsSent() const;
    std::uint64_t totalDataReceived() const;
    std::uint64_t totalPacketsReceived() const;

    bool tryBroadcastAnnouncement(const std::span<std::uint8_t> payload, bool reliable, bool unsequenced);
protected:
    friend Instance;
    friend MessageDispatcher;

    enum class DisconnectType : std::uint8_t
    {
        None,
        Later,
        Force
    };

    struct QueuedEvent
    {
        // Target peer, nullptr for everyone
        ENetPeer* peer;

        // Packet for peer, nullptr on message will discard the event
        ENetPacket* packet;

        // Channel to send packet on
        THORQ_CHANNEL channel;

        // For disconnects
        DisconnectType disconnect;
        THORQ_DISCONNECT_REASON reason;
    };

    bool tryGetEvent(ENetEvent& event, moodycamel::ConsumerToken& token);
    bool tryQueueMessage(ENetPeer* peer, ENetPacket* packet, THORQ_CHANNEL channel, const moodycamel::ProducerToken& token);
    bool tryQueueDisconnect(ENetPeer* peer, bool force, THORQ_DISCONNECT_REASON reason, const moodycamel::ProducerToken& token);
private:
    void run();

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

    moodycamel::ConcurrentQueue<ENetEvent> m_rxQueue;
    moodycamel::ConsumerToken m_rxToken;

    moodycamel::ConcurrentQueue<QueuedEvent> m_txQueue;
    moodycamel::ProducerToken m_txToken;
};
}

#endif // SERVER_H
