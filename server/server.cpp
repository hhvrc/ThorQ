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

#include "server.h"

#include <fmt/core.h>
#include <thorq_message.h>
#include <schemas/version_generated.h>
#include <schemas/heartbeat_generated.h>

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
    , m_heartbeatInterval(15000)
    , m_totalSentData(0)
    , m_totalSentPackets(0)
    , m_totalReceivedData(0)
    , m_totalReceivedPackets(0)
    , m_rxQueue()
    , m_rxToken(m_rxQueue)
    , m_txQueue()
    , m_txToken(m_txQueue)
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

uint32_t ThorQ::Server::heartbeatInterval() const
{
    return m_heartbeatInterval;
}
void ThorQ::Server::setHeartbeatInterval(std::uint32_t msInterval)
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

bool ThorQ::Server::tryBroadcastAnnouncement(const std::span<std::uint8_t> payload, bool reliable, bool unsequenced)
{
    ENetPacket* packet = ThorQ::Memory::packetGet(ThorQ::calculatePacketSize(payload.size(), false));

    if (packet != nullptr)
    {
        packet->flags |= ENET_PACKET_FLAG_RELIABLE * reliable;
        packet->flags |= ENET_PACKET_FLAG_UNSEQUENCED * unsequenced;

        ThorQ::packetEncode(packet, payload);

        m_txQueue.enqueue(Server::QueuedEvent{ nullptr,
                                               packet,
                                               THORQ_CHANNEL::AUTHORITY,
                                               DisconnectType::None,
                                               THORQ_DISCONNECT_REASON::UNKNOWN
                                             });
    }

    return false;
}

bool ThorQ::Server::tryGetEvent(ENetEvent& event, moodycamel::ConsumerToken& token)
{
    return m_rxQueue.try_dequeue(token, event);
}
bool ThorQ::Server::tryQueueMessage(ENetPeer* peer, ENetPacket* packet, THORQ_CHANNEL channel, const moodycamel::ProducerToken& token)
{
    return m_txQueue.enqueue(token, Server::QueuedEvent{ peer,
                                                         packet,
                                                         channel,
                                                         DisconnectType::None,
                                                         THORQ_DISCONNECT_REASON::UNKNOWN
                                                        });
}

bool ThorQ::Server::tryQueueDisconnect(ENetPeer* peer, bool force, THORQ_DISCONNECT_REASON reason, const moodycamel::ProducerToken& token)
{
    return m_txQueue.enqueue(token, Server::QueuedEvent{ peer,
                                                         nullptr,
                                                         THORQ_CHANNEL::_INVALID,
                                                         force ? DisconnectType::Now : DisconnectType::Later,
                                                         reason
                                                        });
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
            if (!m_rxQueue.enqueue(m_rxToken, event))
            {
                fmt::print("Failed to enqueue");
            }
        }

        QueuedEvent queuedMessage;
        while (m_txQueue.try_dequeue(m_txToken, queuedMessage))
        {
            if (queuedMessage.peer != nullptr)
            {
                if (queuedMessage.packet != nullptr && queuedMessage.channel != THORQ_CHANNEL::_INVALID)
                {
                    enet_peer_send(queuedMessage.peer, (std::uint8_t)queuedMessage.channel, queuedMessage.packet);
                }

                switch (queuedMessage.disconnect) {
                case ThorQ::Server::DisconnectType::Now:
                    enet_peer_disconnect_now(queuedMessage.peer, (std::uint32_t)queuedMessage.reason);
                    break;
                case ThorQ::Server::DisconnectType::Later:
                    enet_peer_disconnect_later(queuedMessage.peer, (std::uint32_t)queuedMessage.reason);
                    break;
                case ThorQ::Server::DisconnectType::Request:
                    enet_peer_disconnect(queuedMessage.peer, (std::uint32_t)queuedMessage.reason);
                    break;
                case ThorQ::Server::DisconnectType::None:
                default:
                    break;
                }
            }
            else
            {
                enet_host_broadcast(m_host, (std::uint8_t)queuedMessage.channel, queuedMessage.packet);
            }
        }

        if (++iterations > 1000)
        {
            iterations = 0;

            m_totalSentData += m_host->totalSentData;
            m_host->totalSentData = 0;

            m_totalSentPackets += m_host->totalSentPackets;
            m_host->totalSentPackets = 0;

            m_totalReceivedData += m_host->totalReceivedData;
            m_host->totalReceivedData = 0;

            m_totalReceivedPackets += m_host->totalReceivedPackets;
            m_host->totalReceivedPackets = 0;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    m_status = ServerStatus::Stopped;
}
