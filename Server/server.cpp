#include "server.h"

#include <QThread>
#include <QDebug>

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

#include <thorq_payload_version.h>
#include <thorq_payload_heartbeat.h>

#include "utils.h"
#include "account.h"
#include "instance.h"

bool enet_was_initialized = false;

QString ThorQ::Server::version()
{
    return QString("ENet-%1.%2.%3")
            .arg(ENET_VERSION_MAJOR)
            .arg(ENET_VERSION_MINOR)
            .arg(ENET_VERSION_PATCH);
}
bool ThorQ::Server::Init()
{
    if (!enet_was_initialized && enet_initialize() < 0)
    {
        qWarning() << "Failed to initialize ENet\n";
        return false;
    }
    enet_was_initialized = true;
    return false;
}
void ThorQ::Server::DeInit()
{
    enet_deinitialize();
    enet_was_initialized = false;
}

ThorQ::Server::Server(QObject *parent)
    : QThread(parent)
{
}

bool ThorQ::Server::setup(std::uint16_t port, std::size_t maxPeers, std::uint8_t channelCount)
{

    if (m_host == nullptr)
    {
        ENetAddress address;
        address.host = ENET_HOST_ANY;
        address.port = port;

        m_host = enet_host_create(&address, maxPeers, channelCount, 0, 0); // two channels: communication(tcp), and commands(udp)

        if (m_host == nullptr)
        {
            qWarning() << "An error occurred while trying to create an ENet server host";
            return EXIT_FAILURE;
        }
    }

    return true;
}
void ThorQ::Server::cleanup()
{
    if (m_host != nullptr)
    {
        for (ENetPeer* peer : m_peers)
            enet_peer_disconnect(peer, THORQ_DISCONNECT_REASON_SHUTDOWN_CLOSED);

        ENetEvent event;
        while (m_peers.size() != 0)
        {
            if (enet_host_service(m_host, &event, 0) > 0)
            {
                switch (event.type)
                {
                case ENET_EVENT_TYPE_CONNECT:
                    event.peer->data = nullptr;
                    enet_peer_disconnect_now(event.peer, THORQ_DISCONNECT_REASON_SHUTDOWN_CLOSED);
                    break;
                case ENET_EVENT_TYPE_RECEIVE:
                    enet_packet_destroy(event.packet);
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                {
                    if (event.peer->data != nullptr)
                    {
                        (reinterpret_cast<ThorQ::Instance*>(event.peer->data))->setPeer(nullptr);
                    }
                    enet_peer_reset(event.peer);
                    auto it = std::find(m_peers.begin(), m_peers.end(), event.peer);
                    if (it != m_peers.end())
                    {
                        m_peers.erase(it);
                    }
                    break;
                }
                case ENET_EVENT_TYPE_NONE:
                    break;
                }
            }
        }

        enet_host_destroy(m_host);
        m_host = nullptr;
    }
}

void ThorQ::Server::setNoDelay(bool enabled)
{
    enet_socket_set_option(m_host->socket, ENET_SOCKOPT_NODELAY, enabled);
}

uint16_t ThorQ::Server::heartbeatInterval() const
{
    return m_heartbeatInterval;
}

void ThorQ::Server::setMaxPacketSize(size_t size)
{
    m_host->maximumPacketSize = size;
}
void ThorQ::Server::setHearbeatInterval(uint16_t interval)
{
    m_heartbeatInterval = interval;
}

void ThorQ::Server::handleEventNewConnection(const ENetEvent& event)
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

    auto it = std::find(m_peers.begin(), m_peers.end(), event.peer);
    if (it != m_peers.end())
    {
        m_peers.erase(it);
    }
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


void ThorQ::Server::run()
{
    ENetEvent event;
    std::uint16_t iterations = 0;
    while (!isInterruptionRequested())
    {
        while (enet_host_service(m_host, &event, 0) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                m_peers.insert(event.peer);
                handleEventNewConnection(event);
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                emit enetEvent(event);
                enet_packet_destroy(event.packet);
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
    }
}
