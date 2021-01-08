#include "client.h"

#include <array>
#include <string>

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

#include <QTime>
#include <QTimer>
#include <QThread>
#include <QDebug>
#include <QElapsedTimer>

#include <enums.h>
#include <crypto.h>
#include <version.h>
#include <systemid.h>
#include <constants.h>
#include <thorq_message.h>

static std::string enetaddr_to_str(const ENetAddress* addr)
{
    char buffer[50];
    if (enet_address_get_host_ip(addr, buffer, 50) < 0)
        return "ERROR";
    return std::string(buffer);
}

#define VER_STRING(MAJOR, MINOR, PATCH) #MAJOR "." #MINOR "." #PATCH
const char* ThorQ::Client::Version()
{
    return "ENet-" VER_STRING(ENET_VERSION_MAJOR, ENET_VERSION_MINOR, ENET_VERSION_PATCH);
}

std::atomic_bool g_initialized = false;
bool ThorQ::Client::Initialize()
{
    if (!g_initialized)
    {
        g_initialized = (enet_initialize() == 0);
    }

    return g_initialized;
}
void ThorQ::Client::DeInitialize()
{
    if (g_initialized)
    {
        g_initialized = false;
        enet_deinitialize();
    }
}

ThorQ::Client* ThorQ::Client::NewClient(std::uint8_t channelLimit, QObject* parent)
{
    if (ThorQ::Client::Initialize())
    {
        ENetHost* host = enet_host_create(nullptr, 1, channelLimit, 0, 0);

        if (host != nullptr)
        {
            return new ThorQ::Client(host, parent);
        }
    }

    return nullptr;
}

ThorQ::Client* ThorQ::Client::NewClient(QString hostname, std::uint16_t port, std::size_t peerLimit, std::uint8_t channelLimit, QObject *parent)
{
    if (ThorQ::Client::Initialize())
    {
        ENetAddress addr;

        if (enet_address_set_host(&addr, hostname.toStdString().c_str()) == 0)
        {
            addr.port = port;

            ENetHost* host = enet_host_create(&addr, peerLimit, channelLimit, 0, 0);

            if (host != nullptr)
            {
                return new ThorQ::Client(host, parent);
            }
        }
    }

    return nullptr;
}

ThorQ::Client::Client(ENetHost* host, QObject* parent)
    : QObject(parent)
    , m_serviceTimer(this)
    , m_statisticsTimer(this)
    , m_connections()
    , m_host(host)
{
    // Service timer
    QObject::connect(&m_serviceTimer, &QTimer::timeout, this, &ThorQ::Client::service);
    m_serviceTimer.setSingleShot(false);
    m_serviceTimer.setInterval(5);
    m_serviceTimer.start();

    // Statistics timer
    QObject::connect(&m_statisticsTimer, &QTimer::timeout, this, &ThorQ::Client::updateStats);
    m_statisticsTimer.setSingleShot(false);
    m_statisticsTimer.setInterval(250);
    m_statisticsTimer.start();
}

ThorQ::Client::~Client()
{
    m_statisticsTimer.stop();
    m_serviceTimer.stop();
    m_connections.clear();

    if (m_host != nullptr)
    {
        enet_host_destroy(m_host);
    }
}

QList<ThorQ::ClientConnection*> ThorQ::Client::connections()
{
    return m_connections;
}

std::uint64_t ThorQ::Client::txData() const
{
    return m_totalDataTx;
}

std::uint32_t ThorQ::Client::txSpeed() const
{
    return m_speedTx;
}

std::uint64_t ThorQ::Client::rxData() const
{
    return m_totalDataRx;
}

std::uint32_t ThorQ::Client::rxSpeed() const
{
    return m_speedRx;
}

void ThorQ::Client::connect(QUuid connectionID, QString hostname, std::uint16_t port, std::uint8_t channelCount)
{
    ENetAddress addr;

    if (enet_address_set_host(&addr, hostname.toStdString().c_str()) != 0)
    {
        emit error("Failed to resolve hostname \"" + hostname + "\"");
        return;
    }

    addr.port = port;

    ENetPeer* peer = enet_host_connect(m_host, &addr, channelCount, 0);

    if (peer != nullptr)
    {
        // This is ok
        new ThorQ::ClientConnection(connectionID, peer, this);
    }
}

void ThorQ::Client::service()
{
    ENetEvent event;
    while (enet_host_service(m_host, &event, 0) > 0)
    {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            handleEventConnection(event);
            continue;
        case ENET_EVENT_TYPE_RECEIVE:
            handleEventMessage(event);
            continue;
        case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
            handleEventTimeout(event);
            continue;
        case ENET_EVENT_TYPE_DISCONNECT:
            handleEventDisconnect(event);
            continue;
        case ENET_EVENT_TYPE_NONE:
        default:
            continue;
        }
    }
}

void ThorQ::Client::updateStats()
{
    std::uint32_t incTxData = m_host->totalSentData;
    std::uint32_t incRxData = m_host->totalReceivedData;
    std::uint32_t newTxSpeed = m_host->outgoingBandwidth;
    std::uint32_t newRxSpeed = m_host->incomingBandwidth;

    if (incTxData != 0)
    {
        m_totalDataTx += incTxData;
        m_host->totalSentData = 0;
        emit txDataChanged(m_totalDataTx);
    }

    if (incRxData != 0)
    {
        m_totalDataRx += incRxData;
        m_host->totalReceivedData = 0;
        emit rxDataChanged(m_totalDataRx);
    }

    if (m_speedTx != newTxSpeed)
    {
        m_speedTx = newTxSpeed;
        emit txSpeedChanged(newTxSpeed);
    }

    if (m_speedRx != newRxSpeed)
    {
        m_speedRx = newRxSpeed;
        emit rxSpeedChanged(newRxSpeed);
    }
}

void ThorQ::Client::handleEventConnection(const ENetEvent& event)
{
    ThorQ::ClientConnection* connection;

    if (event.peer != nullptr)
    {
        connection = reinterpret_cast<ThorQ::ClientConnection*>(event.peer->data);

        if (connection == nullptr)
        {
            connection = new ThorQ::ClientConnection(QUuid::createUuid(), event.peer, this);

            QObject::connect(&m_statisticsTimer, &QTimer::timeout, connection, &ThorQ::ClientConnection::updateStats);
            m_connections.push_back(connection);
            emit connectionIncoming(connection);
        }
        else
        {
            QObject::connect(&m_statisticsTimer, &QTimer::timeout, connection, &ThorQ::ClientConnection::updateStats);
            m_connections.push_back(connection);
            emit connectionEstablished(connection);
        }
    }
}

void ThorQ::Client::handleEventMessage(const ENetEvent& event)
{
    auto connection = reinterpret_cast<ClientConnection*>(event.peer->data);

    if (connection != nullptr)
    {
        connection->handleEventMessage(event);
    }
}

void ThorQ::Client::handleEventTimeout(const ENetEvent& event)
{
    auto connection = reinterpret_cast<ClientConnection*>(event.peer->data);

    if (connection != nullptr)
    {
        connection->handleEventTimeout(event);
    }

    handleEventDisconnect(event);
}

void ThorQ::Client::handleEventDisconnect(const ENetEvent &event)
{
    auto connection = reinterpret_cast<ClientConnection*>(event.peer->data);

    if (connection != nullptr)
    {
        if (!m_connections.removeOne(connection))
        {
            emit connectionFailed(connection->id());
        }

        connection->clear();
        QObject::disconnect(&m_statisticsTimer, &QTimer::timeout, connection, &ThorQ::ClientConnection::updateStats);
        connection->handleEventDisconnect(event);
        connection->deleteLater();
    }
}


ThorQ::ClientConnection::ClientConnection(QUuid id, ENetPeer* peer, ThorQ::Client* client)
    : QObject(client)
    , m_id(id)
    , m_peer(peer)
    , m_totalDataTx(0)
    , m_totalDataRx(0)
    , m_speedTx(0)
    , m_speedRx(0)
    , m_rtt(0)
{
    m_peer->data = this;
}

ThorQ::ClientConnection::~ClientConnection()
{
    clear();
}

QUuid ThorQ::ClientConnection::id() const
{
    return m_id;
}

ENetPeer* ThorQ::ClientConnection::peer() const
{
    return m_peer;
}

void ThorQ::ClientConnection::clear()
{
    if (m_peer != nullptr)
    {
        m_peer->data = nullptr;
        m_peer = nullptr;

        m_totalDataTx = 0;
        m_totalDataRx = 0;
        m_speedTx = 0;
        m_speedRx = 0;
        m_rtt = 0;
    }
}

void ThorQ::ClientConnection::updateStats()
{
    if (m_peer == nullptr) return;

    std::uint16_t newRtt = m_peer->roundTripTime;
    std::uint32_t incTxData = m_peer->totalDataSent;
    std::uint32_t incRxData = m_peer->totalDataReceived;
    std::uint32_t newTxSpeed = m_peer->outgoingBandwidth;
    std::uint32_t newRxSpeed = m_peer->incomingBandwidth;

    if (m_rtt != newRtt)
    {
        m_rtt = newRtt;
        emit rttChanged(newRtt);
    }

    if (incTxData != 0)
    {
        m_totalDataTx += incTxData;
        m_peer->totalDataSent = 0;
        emit txDataChanged(m_totalDataTx);
    }

    if (incRxData != 0)
    {
        m_totalDataRx += incRxData;
        m_peer->totalDataReceived = 0;
        emit rxDataChanged(m_totalDataRx);
    }

    if (m_speedTx != newTxSpeed)
    {
        m_speedTx = newTxSpeed;
        emit txSpeedChanged(newTxSpeed);
    }

    if (m_speedRx != newRxSpeed)
    {
        m_speedRx = newRxSpeed;
        emit rxSpeedChanged(newRxSpeed);
    }
}

void ThorQ::ClientConnection::handleEventMessage(const ENetEvent& event)
{
    emit packetReceived(ThorQ::ClientMessage(event.packet, event.channelID));
}

void ThorQ::ClientConnection::handleEventTimeout(const ENetEvent& event)
{
    Q_UNUSED(event)
    emit disconnected((std::uint32_t)THORQ_DISCONNECT_REASON::TIMED_OUT);
}

void ThorQ::ClientConnection::handleEventDisconnect(const ENetEvent& event)
{
    emit disconnected(event.data);
}

std::uint64_t ThorQ::ClientConnection::txData() const
{
    return m_totalDataTx;
}

std::uint32_t ThorQ::ClientConnection::txSpeed() const
{
    return m_speedTx;
}

std::uint64_t ThorQ::ClientConnection::rxData() const
{
    return m_totalDataRx;
}

std::uint32_t ThorQ::ClientConnection::rxSpeed() const
{
    return m_speedRx;
}

void ThorQ::ClientConnection::packetSend(QByteArray data, uint8_t channel)
{
    sendPacket(data, ENET_PACKET_FLAG_UNSEQUENCED, channel);
}

void ThorQ::ClientConnection::packetSendReliable(QByteArray data, uint8_t channel)
{
    sendPacket(data, ENET_PACKET_FLAG_RELIABLE, channel);
}

void ThorQ::ClientConnection::packetSendSequenced(QByteArray data, uint8_t channel)
{
    sendPacket(data, 0, channel);
}

void ThorQ::ClientConnection::disconnect(std::uint32_t reason)
{
    enet_peer_disconnect(m_peer, reason);
}

void ThorQ::ClientConnection::disconnectNow(std::uint32_t reason)
{
    enet_peer_disconnect_now(m_peer, reason);
}

void ThorQ::ClientConnection::disconnectLater(std::uint32_t reason)
{
    enet_peer_disconnect_later(m_peer, reason);
}

void ThorQ::ClientConnection::sendPacket(const QByteArray &data, std::uint32_t flags, uint8_t channel)
{
    if (m_peer == nullptr)
    {
        // ERROR
        return;
    }

    ENetPacket* packet = enet_packet_create(data.data(), data.size(), flags);

    if (packet == nullptr)
    {
        // ERROR
        return;
    }

    if (enet_peer_send(m_peer, channel, packet) == -1)
    {
        enet_packet_destroy(packet);
        // ERROR
        return;
    }
}

ThorQ::ClientMessage::ClientMessage(ENetPacket* packet, std::uint8_t channelID)
    : m_packet(packet, [](ENetPacket* packet){ enet_packet_destroy(packet); })
    , m_channelID(channelID)
{
}

ThorQ::ClientMessage::ClientMessage()
    : m_packet(nullptr)
    , m_channelID(0)
{
}

ThorQ::ClientMessage::ClientMessage(const ThorQ::ClientMessage &other)
    : m_packet(other.m_packet)
    , m_channelID(other.m_channelID)
{
}

ThorQ::ClientMessage& ThorQ::ClientMessage::operator=(const ThorQ::ClientMessage& other)
{
    m_packet = other.m_packet;
    m_channelID = other.m_channelID;
    return *this;
}

ThorQ::ClientMessage::~ClientMessage()
{
}

std::shared_ptr<ENetPacket> ThorQ::ClientMessage::packet() const
{
    return m_packet;
}

std::uint8_t ThorQ::ClientMessage::channelID() const
{
    return m_channelID;
}
