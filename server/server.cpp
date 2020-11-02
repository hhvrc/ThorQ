#include "server.h"

#if defined(__GCC__) || defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif

#define ENET_IMPLEMENTATION
#include <enet.h>

#if defined(__GCC__) || defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#include <fmt/core.h>
#include <thorq_message.h>
#include <schemas/version_generated.h>
#include <schemas/heartbeat_generated.h>

#include "utils.h"
#include "account.h"
#include "instance.h"
#include "memorymanager.h"
#include "messagedispatcher.h"

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
    , m_rxToken(m_rxQueue)
    , m_rxQueue()
    , m_txToken(m_txQueue)
    , m_txQueue()
    , m_broadcastToken(m_broadcastQueue)
    , m_broadcastQueue()
    , m_disconnectToken(m_disconnectQueue)
    , m_disconnectQueue()
{
}
ThorQ::Server::~Server()
{
    if (m_host == nullptr) return;

    stop();

    try
    {
        // Disconnect all clients
        ENetPeer* firstPeer = m_host->peers;
        ENetPeer* lastPeer  = m_host->peers + m_host->peerCount;

        for (ENetPeer* peer = firstPeer; peer != lastPeer; peer++)
        {
            enet_peer_disconnect_now(peer, (std::uint32_t)THORQ_DISCONNECT_REASON::SHUTDOWN_CLOSED);
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
    if (m_thread != nullptr) return true;

    try
    {
        ENetAddress address;
        address.host = ENET_HOST_ANY;
        address.port = port;

        if (m_host != nullptr)
        {
            enet_host_destroy(m_host);
            m_host = nullptr;
        }

        m_host = enet_host_create(&address, maxPeers, channelCount, 0, 0);

        if (m_host != nullptr)
        {
            enet_socket_set_option(m_host->socket, ENET_SOCKOPT_NODELAY, noDelay);

            m_host->maximumPacketSize = 65536; // 64kB (enough to hold a 80x80 rgba image, and enough to hold a compiled arduino program)

            m_run = true;

            m_thread = new std::thread(&ThorQ::Server::run, this);

            unsigned int nProc = std::thread::hardware_concurrency();
            for (unsigned int i = 0; i < nProc; i++)
            {
                m_dispatchers.push_back(new ThorQ::MessageDispatcher(this));
            }

            return true;
        }
    }
    catch (...)
    {
        if (m_host != nullptr)
        {
            enet_host_destroy(m_host);
            m_host = nullptr;
        }
        if (m_thread != nullptr)
        {
            delete m_thread;
            m_thread = nullptr;
        }
    }

    return false;
}

void ThorQ::Server::stop()
{
    for (std::size_t i = 0; i < m_dispatchers.size(); i++)
    {
        delete m_dispatchers[i];
    }
    m_dispatchers.clear();

    if (m_thread != nullptr)
    {
        m_run = false;
        m_thread->join();
        delete  m_thread;
        m_thread = nullptr;
    }
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

void ThorQ::Server::broadcastAnnouncement(const flatbuffers::DetachedBuffer& payload, bool reliable, bool unsequenced)
{
    ENetPacket* packet = ThorQ::Memory::packetGet(payload.size(), (ENET_PACKET_FLAG_RELIABLE * reliable) | (ENET_PACKET_FLAG_UNSEQUENCED * unsequenced));

    if (packet != nullptr)
    {
        ThorQ::packetEncode(packet, payload.data(), payload.size());
        m_broadcastQueue.enqueue(packet);
    }
}

bool ThorQ::Server::tryGetMessage(moodycamel::ConsumerToken token, ThorQ::Server::QueuedMessage &message)
{
    return m_rxQueue.try_dequeue(token, message);
}
bool ThorQ::Server::tryQueueMessage(moodycamel::ProducerToken token, const QueuedMessage& message)
{
    return m_txQueue.enqueue(token, message);
}

bool ThorQ::Server::disconnectPeer(moodycamel::ProducerToken token, QueuedDisconnect &disconnect)
{
    return m_disconnectQueue.enqueue(token, disconnect);
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
                goto disconnect; // To avoid [-Wimplicit-fallthrough]
            case ENET_EVENT_TYPE_DISCONNECT:
            disconnect:
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
/*
    flatbuffers::FlatBufferBuilder builder;
    ThorQ::Serialization::VersionBuilder versionBuilder(builder);
    versionBuilder.

    thorq_payload_version_pack(message, THORQ_APP_LINK, THORQ_VERSION_LINK);
    instance->packetSend(message, THORQ_CHANNEL_MAIN, false, true);

    thorq_payload_version_pack(message, THORQ_APP_CLIENT, THORQ_VERSION_CLIENT);
    instance->packetSend(message, THORQ_CHANNEL_MAIN, false, true);

    thorq_payload_version_pack(message, THORQ_APP_SERVER, THORQ_VERSION_SERVER);
    instance->packetSend(message, THORQ_CHANNEL_MAIN, false, true);

    thorq_payload_heartbeat_pack(message, 500); // TODO: get from config
    instance->packetSend(message, THORQ_CHANNEL_MAIN, false, true);
*/
    fmt::print("[{}] Connected\n", enet_peer_address_str(event.peer));
}
void ThorQ::Server::handleEventMessage(const ENetEvent &event)
{
    m_rxQueue.enqueue(QueuedMessage{ event.peer, event.packet, event.channelID });
}
void ThorQ::Server::handleEventDisconnect(const ENetEvent& event)
{
    // Get instance
    ThorQ::Instance* instance = reinterpret_cast<ThorQ::Instance*>(event.peer->data);

    // Remove pointer
    event.peer->data = nullptr;

    // Yeet
    delete instance;

    fmt::print("[{}] disconnected\n", enet_peer_address_str(event.peer));
}
void ThorQ::Server::handleEventTimeout(const ENetEvent& event)
{
    if (event.peer->data == nullptr)
        return;

    fmt::print("[{}] timed out\n", enet_peer_address_str(event.peer));
}
