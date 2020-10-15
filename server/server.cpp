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

#include <thorq_message.h>
#include <thorq_payload_version.h>
#include <thorq_payload_heartbeat.h>

#include "utils.h"
#include "account.h"
#include "instance.h"
#include "memorymanager.h"

// ceil(x / y) https://stackoverflow.com/questions/2745074/fast-ceiling-of-an-integer-division-in-c-c
constexpr std::size_t ceilDiv(std::size_t x, std::size_t y)
{
    return (x + y - 1) / y;
}

// https://github.com/cameron314/concurrentqueue
constexpr std::size_t blockSize = moodycamel::ConcurrentQueueDefaultTraits::BLOCK_SIZE;
constexpr std::size_t queueCapicity = 1024;
constexpr std::size_t queueSizeIn  = (ceilDiv(queueCapicity, blockSize) + 1) * 16 * blockSize;
constexpr std::size_t queueSizeOut = (ceilDiv(queueCapicity, blockSize) + 1) *  1 * blockSize;

ENetPacket* allocPacket(const void *data, size_t dataLength, enet_uint32 flags)
{

}
void freePacket(ENetPacket *packet)
{

}

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

ThorQ::Server::Server(std::uint16_t port, std::size_t maxPeers, std::uint8_t channelCount, bool noDelay)
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
ThorQ::Server::~Server()
{
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
        qDebug() << "Exception occured disconnecting clients:" << ex.what();
    }
    catch (int i)
    {
        qDebug() << "Exception occured disconnecting clients:" << strerror(i);
    }
    catch (...)
    {

    }

    try
    {
        enet_host_destroy(m_host);
    }
    catch (const std::exception& ex)
    {
        qDebug() << "Exception occured destroying host:" << ex.what();
    }
    catch (int i)
    {
        qDebug() << "Exception occured destroying host:" << strerror(i);
    }
    catch (...)
    {

    }
}

bool ThorQ::Server::ready()
{
    return m_host != nullptr;
}

uint32_t ThorQ::Server::HeartbeatInterval()
{
    return m_heartbeatInterval;
}
void ThorQ::Server::SetHeartbeatInterval(std::uint32_t msInterval)
{
    if (msInterval != m_heartbeatInterval)
    {
        m_heartbeatInterval = msInterval;
        emit heartbeatChanged(msInterval);
    }
}

std::uint64_t ThorQ::Server::totalDataSent()
{
    return m_totalSentData;
}
std::uint64_t ThorQ::Server::totalPacketsSent()
{
    return m_totalSentPackets;
}
std::uint64_t ThorQ::Server::totalDataReceived()
{
    return m_totalReceivedData;
}
std::uint64_t ThorQ::Server::totalPacketsReceived()
{
    return m_totalReceivedPackets;
}

void ThorQ::Server::broadcastAnnouncement(const std::vector<std::uint8_t>& payload, bool reliable, bool unsequenced)
{
    ENetPacket* packet;
    ThorQ::packetEncode(payload, (ENET_PACKET_FLAG_RELIABLE * reliable) | (ENET_PACKET_FLAG_UNSEQUENCED * unsequenced));

    if (packet != nullptr)
    {
        m_queuedBroadcasts.enqueue(packet);
    }
}

bool ThorQ::Server::tryGetMessage(ThorQ::Server::QueuedMessage &message)
{
    return m_receivedMessages.try_dequeue(message);
}
void ThorQ::Server::queueMessage(const QueuedMessage& message)
{
    m_queuedMessages.enqueue(message);
}

void ThorQ::Server::run()
{
    ENetEvent event;
    std::uint16_t iterations = 0;

    setStatus(ServerStatus::Started);

    while (m_shouldRun)
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

            ENetPacket* queuedBroadcast;
            if (m_queuedBroadcasts.try_dequeue(queuedBroadcast))
            {
                enet_host_broadcast(m_host, THORQ_CHANNEL_AUTHORITY, queuedBroadcast);
            }

            QueuedMessage queuedMessage;
            if (m_queuedMessages.try_dequeue(queuedMessage))
            {
                enet_peer_send(queuedMessage.peer, queuedMessage.channel, queuedMessage.packet);
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
    }

    setStatus(ServerStatus::Stopped);
}

void ThorQ::Server::handleEventConnection(const ENetEvent& event)
{
    // Dont worry, its ok to have a seemingly dangling pointer here (ENet keeps track of the pointer)

    ThorQ::Instance* instance = new ThorQ::Instance(event.peer);

    std::vector<std::uint8_t> message;

    thorq_payload_version_pack(message, THORQ_APP_LINK, THORQ_VERSION_LINK);
    instance->sendMessage(message, THORQ_CHANNEL_MAIN, false, true);

    thorq_payload_version_pack(message, THORQ_APP_CLIENT, THORQ_VERSION_CLIENT);
    instance->sendMessage(message, THORQ_CHANNEL_MAIN, false, true);

    thorq_payload_version_pack(message, THORQ_APP_SERVER, THORQ_VERSION_SERVER);
    instance->sendMessage(message, THORQ_CHANNEL_MAIN, false, true);

    thorq_payload_heartbeat_pack(message, 500); // TODO: get from config
    instance->sendMessage(message, THORQ_CHANNEL_MAIN, false, true);

    qDebug() << QString("A new client connected from:\n\tIPV6: %1\n\tPORT: %2")
                .arg(enet_peer_address_str(event.peer))
                .arg(event.peer->address.port);
}
void ThorQ::Server::handleEventMessage(const ENetEvent &event)
{
    m_receivedMessages.enqueue(QueuedMessage{ event.peer, event.packet, event.channelID });
}
void ThorQ::Server::handleEventDisconnect(const ENetEvent& event)
{
    if (event.peer->data == nullptr)
        return;

    auto instance = reinterpret_cast<ThorQ::Instance*>(event.peer->data);

    if (instance->account() != nullptr)
    {
        qDebug() << "User" << instance->account()->username()
                 << "connected from [" << enet_peer_address_str(event.peer) << "] disconnected";
    }
    else
    {
        qDebug() << "User connected from [" << enet_peer_address_str(event.peer) << "] disconnected";
    }

    event.peer->data = nullptr;

    // Automatically notifies others
    instance->deleteLater();
}
void ThorQ::Server::handleEventTimeout(const ENetEvent& event)
{
    if (event.peer->data == nullptr)
        return;

    ThorQ::Instance* instance = reinterpret_cast<ThorQ::Instance*>(event.peer->data);

    if (instance->account() != nullptr)
    {
        qDebug() << instance->account()->username() << "timed out";
    }
}
