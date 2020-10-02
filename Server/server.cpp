#include "server.h"

#include <cstdint>

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

#include <thorq_message.h>
#include <thorq_payload_version.h>
#include <thorq_payload_heartbeat.h>

#include "utils.h"
#include "account.h"
#include "instance.h"


std::atomic<bool> g_initialized = false;

QString ThorQ::Server::Version()
{
    return QString("ENet-%1.%2.%3")
            .arg(ENET_VERSION_MAJOR)
            .arg(ENET_VERSION_MINOR)
            .arg(ENET_VERSION_PATCH);
}
bool ThorQ::Server::Initialize()
{
    if (!g_initialized && enet_initialize() < 0)
    {
        qWarning() << "Failed to initialize ENet\n";
        return false;
    }
    g_initialized = true;
    return false;
}
void ThorQ::Server::DeInitialize()
{
    enet_deinitialize();
    g_initialized = false;
}

bool ThorQ::Server::setup(uint16_t port, std::size_t maxPeers, uint8_t channelCount, bool noDelay)
{
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    ENetHost* host = enet_host_create(&address, maxPeers, channelCount, 0, 0); // two channels: communication(tcp), and commands(udp)

    if (host == nullptr)
    {
        return false;
    }

    enet_socket_set_option(host->socket, ENET_SOCKOPT_NODELAY, noDelay);

    host->maximumPacketSize = THORQ_MESSAGE_LEN;

    return true;
}

bool ThorQ::Server::start()
{

}

void ThorQ::Server::stop()
{

}

void ThorQ::Server::cleanup()
{
    stop();

    // Disconnect all clients
    ENetPeer* firstPeer = m_host->peers;
    ENetPeer* lastPeer  = m_host->peers + m_host->peerCount;

    for (ENetPeer* peer = firstPeer; peer != lastPeer; peer++)
    {
        enet_peer_disconnect_now(peer, THORQ_DISCONNECT_REASON_SHUTDOWN_CLOSED);
    }

    enet_host_flush(m_host);
    enet_host_destroy(m_host);

    m_host = nullptr;
}




uint16_t ThorQ::Server::HeartbeatInterval()
{
    return m_heartbeatInterval;
}
void ThorQ::Server::SetHeartbeatInterval(uint16_t msInterval)
{
    m_heartbeatInterval = msInterval;
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

bool ThorQ::Server::Start(std::uint16_t port, std::size_t maxPeers, std::uint8_t channelCount)
{

    ENetEvent event;
    ::std::uint16_t iterations = 0;
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

    return true;
}
