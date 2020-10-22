#include "server.h"

#if defined(__GCC__) || defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif

#define ENET_IMPLEMENTATION
#include <enet.h>

#if defined(__GCC__) || defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#include <fmt/core.h>
#include <thorq_message.h>
#include <thorq_payload_version.h>
#include <thorq_payload_heartbeat.h>

#include "utils.h"
#include "account.h"
#include "instance.h"
#include "memorymanager.h"

#define VER_STRING(MAJOR, MINOR, PATCH) #MAJOR "." #MINOR "." #PATCH
const char* ThorQ::Server::Version()
{
    return "ENet-" VER_STRING(ENET_VERSION_MAJOR, ENET_VERSION_MINOR, ENET_VERSION_PATCH);
}


std::atomic<bool> g_initialized = false;
bool ThorQ::Server::Initialize()
{
    if (!g_initialized)
    {
        ENetCallbacks callbacks = ThorQ::Memory::Initialize();

        g_initialized = (enet_initialize_with_callbacks(ENET_VERSION, &callbacks) == 0);
    }
    return g_initialized;
}
void ThorQ::Server::DeInitialize()
{
    if (g_initialized)
    {
        enet_deinitialize();
        g_initialized = false;
    }
}

ThorQ::Server::Server()
    : m_host(nullptr)
    , m_thread(nullptr)
    , m_heartbeatInterval(500)
    , m_totalSentData(0)
    , m_totalSentPackets(0)
    , m_totalReceivedData(0)
    , m_totalReceivedPackets(0)
    , m_txQueue()
    , m_txToken(m_txQueue)
    , m_rxQueue()
    , m_rxToken(m_rxQueue)
    , m_broadcastQueue()
    , m_broadcastToken(m_broadcastQueue)
{
}
ThorQ::Server::~Server()
{
    if (m_host == nullptr) return;

    try
    {
        // Disconnect all clients
        ENetPeer* firstPeer = m_host->peers;
        ENetPeer* lastPeer  = m_host->peers + m_host->peerCount;

        for (ENetPeer* peer = firstPeer; peer != lastPeer; peer++)
        {
            enet_peer_disconnect_now(peer, THORQ_DISCONNECT_REASON_SHUTDOWN_CLOSED);
        }

        enet_host_flush(m_host);
    }
    catch (const std::exception& ex)
    {
        fmt::print(stderr, "Exception occured disconnecting clients: {}\n", ex.what());
    }
    catch (int i)
    {
        char buf[64]{0};
#ifdef _WIN32
        strerror_s(buf, 63, i);
#else
        strerror_r(i, buf, 63);
#endif
        fmt::print(stderr, "Exception occured disconnecting clients: {}\n", buf);
    }
    catch (...)
    {
        fmt::print(stderr, "Unknown Exception occured disconnecting clients\n");
    }

    try
    {
        enet_host_destroy(m_host);
    }
    catch (const std::exception& ex)
    {
        fmt::print(stderr, "Exception occured destroying host: {}\n", ex.what());
    }
    catch (int i)
    {
        char buf[64]{0};
#ifdef _WIN32
        strerror_s(buf, 63, i);
#else
        strerror_r(i, buf, 63);
#endif
        fmt::print(stderr, "Exception occured destroying host: {}\n", buf);
    }
    catch (...)
    {
        fmt::print(stderr, "Unknown Exception occured destroying host\n");
    }
}

ThorQ::Server::ServerStatus ThorQ::Server::status() const
{
    return m_status;
}

bool ThorQ::Server::start(std::uint16_t port, std::size_t maxPeers, std::uint8_t channelCount, bool noDelay)
{
    try
    {
        ENetAddress address;
        address.host = ENET_HOST_ANY;
        address.port = port;

        m_host = enet_host_create(&address, maxPeers, channelCount, 0, 0); // two channels: communication(tcp), and commands(udp)

        if (m_host != nullptr)
        {
            enet_socket_set_option(m_host->socket, ENET_SOCKOPT_NODELAY, noDelay);

            m_host->maximumPacketSize = 65536; // 64kB (enough to hold a 80x80 rgba image, and enough to hold a compiled arduino program)
        }
    }
    catch (...)
    {
        m_host = nullptr;
    }
}

bool ThorQ::Server::stop()
{

}

uint32_t ThorQ::Server::HeartbeatInterval() const
{
    return m_heartbeatInterval;
}
void ThorQ::Server::SetHeartbeatInterval(std::uint32_t msInterval)
{
    if (msInterval != m_heartbeatInterval)
    {
        m_heartbeatInterval = msInterval;
    }
}

std::uint64_t ThorQ::Server::totalDataSent() const
{
    return m_totalSentData;
}
std::uint64_t ThorQ::Server::totalPacketsSent() const
{
    return m_totalSentPackets;
}
std::uint64_t ThorQ::Server::totalDataReceived() const
{
    return m_totalReceivedData;
}
std::uint64_t ThorQ::Server::totalPacketsReceived() const
{
    return m_totalReceivedPackets;
}

void ThorQ::Server::broadcastAnnouncement(const std::vector<std::uint8_t>& payload, bool reliable, bool unsequenced)
{
    ENetPacket* packet = ThorQ::Memory::packetGet(payload.size(), (ENET_PACKET_FLAG_RELIABLE * reliable) | (ENET_PACKET_FLAG_UNSEQUENCED * unsequenced));

    if (packet != nullptr)
    {
        ThorQ::packetEncode(packet, payload);
        m_broadcastQueue.enqueue(QueuedMessage{ nullptr, packet, THORQ_CHANNEL_AUTHORITY });
    }
}

bool ThorQ::Server::tryGetMessage(ThorQ::Server::QueuedMessage &message)
{
    return m_rxQueue.try_dequeue(m_rxToken, message);
}
bool ThorQ::Server::tryQueueMessage(const QueuedMessage& message)
{
    return m_txQueue.enqueue(m_txToken, message);
}

void ThorQ::Server::run()
{
    ENetEvent event;
    std::uint16_t iterations = 0;

    m_status = ServerStatus::Running;

    while (m_status == ServerStatus::Running)
    {
        while (enet_host_service(m_host, &event, 0) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                handleEventConnection(event);
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                handleEventMessage(event);
                break;
            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                handleEventTimeout(event);
            case ENET_EVENT_TYPE_DISCONNECT:
                handleEventDisconnect(event);
                break;
            case ENET_EVENT_TYPE_NONE:
                break;
            }

            if (++iterations > 100)
            {
                m_totalSentData += m_host->totalSentData;
                m_host->totalSentData = 0;

                m_totalSentPackets += m_host->totalSentPackets;
                m_host->totalSentPackets = 0;

                m_totalReceivedData += m_host->totalReceivedData;
                m_host->totalReceivedData = 0;

                m_totalReceivedPackets += m_host->totalReceivedPackets;
                m_host->totalReceivedPackets = 0;
            }
        }

        ENetPacket* queuedBroadcast;
        while (m_broadcastQueue.try_dequeue(queuedBroadcast))
        {
            enet_host_broadcast(m_host, THORQ_CHANNEL_AUTHORITY, queuedBroadcast);
        }

        QueuedMessage queuedMessage;
        while (m_txQueue.try_dequeue(queuedMessage))
        {
            enet_peer_send(queuedMessage.peer, queuedMessage.channel, queuedMessage.packet);
        }
    }

    m_status = ServerStatus::Stopped;
}

void ThorQ::Server::handleEventConnection(const ENetEvent& event)
{
    // Dont worry, its ok to have a seemingly dangling pointer here (ENet keeps track of the pointer)

    ThorQ::Instance* instance = new ThorQ::Instance(event.peer);

    std::vector<std::uint8_t> message;

    thorq_payload_version_pack(message, THORQ_APP_LINK, THORQ_VERSION_LINK);
    instance->sendPayload(message, THORQ_CHANNEL_MAIN, false, true);

    thorq_payload_version_pack(message, THORQ_APP_CLIENT, THORQ_VERSION_CLIENT);
    instance->sendPayload(message, THORQ_CHANNEL_MAIN, false, true);

    thorq_payload_version_pack(message, THORQ_APP_SERVER, THORQ_VERSION_SERVER);
    instance->sendPayload(message, THORQ_CHANNEL_MAIN, false, true);

    thorq_payload_heartbeat_pack(message, 500); // TODO: get from config
    instance->sendPayload(message, THORQ_CHANNEL_MAIN, false, true);

    fmt::print("[{}] Connected", enet_peer_address_str(event.peer));
}
void ThorQ::Server::handleEventMessage(const ENetEvent &event)
{
    m_rxQueue.enqueue(QueuedMessage{ event.peer, event.packet, event.channelID });
}
void ThorQ::Server::handleEventDisconnect(const ENetEvent& event)
{
    if (event.peer->data == nullptr)
        return;

    auto instance = reinterpret_cast<ThorQ::Instance*>(event.peer->data);

    fmt::print("[{}] disconnected\n", enet_peer_address_str(event.peer));

    event.peer->data = nullptr;

    // Automatically notifies others
    instance->deleteLater();
}
void ThorQ::Server::handleEventTimeout(const ENetEvent& event)
{
    if (event.peer->data == nullptr)
        return;

    ThorQ::Instance* instance = reinterpret_cast<ThorQ::Instance*>(event.peer->data);

    fmt::print("[{}] timed out\n", enet_peer_address_str(event.peer));
}
