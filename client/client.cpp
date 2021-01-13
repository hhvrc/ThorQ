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

#define VER_STRING(MAJOR, MINOR, PATCH) #MAJOR "." #MINOR "." #PATCH
const char* ThorQ::Networking::Client::Version()
{
    return "ENet-" VER_STRING(ENET_VERSION_MAJOR, ENET_VERSION_MINOR, ENET_VERSION_PATCH);
}

std::atomic_bool g_initialized = false;
bool ThorQ::Networking::Client::Initialize()
{
    if (!g_initialized)
    {
        g_initialized = (enet_initialize() == 0);
    }

    return g_initialized;
}
void ThorQ::Networking::Client::DeInitialize()
{
    if (g_initialized)
    {
        g_initialized = false;
        enet_deinitialize();
    }
}

ThorQ::Networking::Client* ThorQ::Networking::Client::NewClient(std::uint8_t channelLimit, QObject* parent)
{
    if (ThorQ::Networking::Client::Initialize())
    {
        ENetHost* host = enet_host_create(nullptr, 1, channelLimit, 0, 0);

        if (host != nullptr)
        {
            return new ThorQ::Networking::Client(host, parent);
        }
    }

    return nullptr;
}

ThorQ::Networking::Client* ThorQ::Networking::Client::NewClient(QString hostname, std::uint16_t port, std::size_t peerLimit, std::uint8_t channelLimit, QObject *parent)
{
    if (ThorQ::Networking::Client::Initialize())
    {
        ENetAddress addr;

        if (enet_address_set_host(&addr, hostname.toStdString().c_str()) == 0)
        {
            addr.port = port;

            ENetHost* host = enet_host_create(&addr, peerLimit, channelLimit, 0, 0);


            if (host != nullptr)
            {
                return new ThorQ::Networking::Client(host, parent);
            }
        }
    }

    return nullptr;
}

ThorQ::Networking::Client::Client(ENetHost* host, QObject* parent)
    : QObject(parent)
    , m_serviceTimer(this)
    , m_statisticsTimer(this)
    , m_connections()
    , m_host(host)
{
    // Service timer
    QObject::connect(&m_serviceTimer, &QTimer::timeout, this, &ThorQ::Networking::Client::service);
    m_serviceTimer.setSingleShot(false);
    m_serviceTimer.setInterval(5);
    m_serviceTimer.start();

    // Statistics timer
    QObject::connect(&m_statisticsTimer, &QTimer::timeout, this, &ThorQ::Networking::Client::updateStats);
    m_statisticsTimer.setSingleShot(false);
    m_statisticsTimer.setInterval(250);
    m_statisticsTimer.start();
}

ThorQ::Networking::Client::~Client()
{
    m_statisticsTimer.stop();
    m_serviceTimer.stop();
    m_connections.clear();

    if (m_host != nullptr)
    {
        enet_host_destroy(m_host);
    }
}

QList<ThorQ::Networking::Connection*> ThorQ::Networking::Client::connections()
{
    return m_connections;
}

std::uint64_t ThorQ::Networking::Client::txData() const
{
    return m_totalDataTx;
}

std::uint32_t ThorQ::Networking::Client::txSpeed() const
{
    return m_speedTx;
}

std::uint64_t ThorQ::Networking::Client::rxData() const
{
    return m_totalDataRx;
}

std::uint32_t ThorQ::Networking::Client::rxSpeed() const
{
    return m_speedRx;
}

void ThorQ::Networking::Client::connect(QUuid connectionID, QString hostname, std::uint16_t port, std::uint8_t channelCount)
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
        new ThorQ::Networking::Connection(connectionID, peer, this);
    }
}

void ThorQ::Networking::Client::service()
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

void ThorQ::Networking::Client::updateStats()
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

void ThorQ::Networking::Client::handleEventConnection(const ENetEvent& event)
{
    ThorQ::Networking::Connection* connection;

    if (event.peer != nullptr)
    {
        connection = reinterpret_cast<ThorQ::Networking::Connection*>(event.peer->data);

        if (connection == nullptr)
        {
            connection = new ThorQ::Networking::Connection(QUuid::createUuid(), event.peer, this);

            QObject::connect(&m_statisticsTimer, &QTimer::timeout, connection, &ThorQ::Networking::Connection::updateStats);
            m_connections.push_back(connection);
            emit connectionIncoming(connection);
        }
        else
        {
            QObject::connect(&m_statisticsTimer, &QTimer::timeout, connection, &ThorQ::Networking::Connection::updateStats);
            m_connections.push_back(connection);
            emit connectionEstablished(connection);
        }
    }
}

void ThorQ::Networking::Client::handleEventMessage(const ENetEvent& event)
{
    auto connection = reinterpret_cast<Connection*>(event.peer->data);

    if (connection != nullptr)
    {
        connection->handleEventMessage(event);
    }
}

void ThorQ::Networking::Client::handleEventTimeout(const ENetEvent& event)
{
    auto connection = reinterpret_cast<Connection*>(event.peer->data);

    if (connection != nullptr)
    {
        connection->handleEventTimeout(event);
    }

    handleEventDisconnect(event);
}

void ThorQ::Networking::Client::handleEventDisconnect(const ENetEvent &event)
{
    auto connection = reinterpret_cast<Connection*>(event.peer->data);

    if (connection != nullptr)
    {
        if (!m_connections.removeOne(connection))
        {
            emit connectionFailed(connection->id());
        }

        connection->clear();
        QObject::disconnect(&m_statisticsTimer, &QTimer::timeout, connection, &ThorQ::Networking::Connection::updateStats);
        connection->handleEventDisconnect(event);
        connection->deleteLater();
    }
}


ThorQ::Networking::Connection::Connection(QUuid id, ENetPeer* peer, ThorQ::Networking::Client* client)
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

ThorQ::Networking::Connection::~Connection()
{
    clear();
}

QUuid ThorQ::Networking::Connection::id() const
{
    return m_id;
}

ENetPeer* ThorQ::Networking::Connection::peer() const
{
    return m_peer;
}

void ThorQ::Networking::Connection::clear()
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

void ThorQ::Networking::Connection::updateStats()
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

void ThorQ::Networking::Connection::handleEventMessage(const ENetEvent& event)
{
    emit udpReceived(ThorQ::Networking::Message(event.packet, event.channelID));
}

void ThorQ::Networking::Connection::handleEventTimeout(const ENetEvent& event)
{
    Q_UNUSED(event)
    emit disconnected((std::uint32_t)THORQ_DISCONNECT_REASON::TIMED_OUT);
}

void ThorQ::Networking::Connection::handleEventDisconnect(const ENetEvent& event)
{
    emit disconnected(event.data);
}

std::uint64_t ThorQ::Networking::Connection::txData() const
{
    return m_totalDataTx;
}

std::uint32_t ThorQ::Networking::Connection::txSpeed() const
{
    return m_speedTx;
}

std::uint64_t ThorQ::Networking::Connection::rxData() const
{
    return m_totalDataRx;
}

std::uint32_t ThorQ::Networking::Connection::rxSpeed() const
{
    return m_speedRx;
}

void ThorQ::Networking::Connection::udpSend(ThorQ::Networking::Message message)
{
    if (m_peer == nullptr)
    {
        // ERROR
        return;
    }

    if (enet_peer_send(m_peer, message.channelID(), message.packet()) == -1)
    {
        // ERROR
        return;
    }
}

void ThorQ::Networking::Connection::disconnect(std::uint32_t reason)
{
    enet_peer_disconnect(m_peer, reason);
}

void ThorQ::Networking::Connection::disconnectNow(std::uint32_t reason)
{
    enet_peer_disconnect_now(m_peer, reason);
}

void ThorQ::Networking::Connection::disconnectLater(std::uint32_t reason)
{
    enet_peer_disconnect_later(m_peer, reason);
}

ThorQ::Networking::Message::Message()
    : m_packet(nullptr)
    , m_channelID(0)
{
}

ThorQ::Networking::Message::Message(const ThorQ::Networking::Message &other)
    : m_packet(other.m_packet)
    , m_channelID(other.m_channelID)
{
}

ThorQ::Networking::Message::Message(ENetPacket* packet, std::uint8_t channelID)
    : m_packet(packet, [](ENetPacket* packet){ enet_packet_destroy(packet); })
    , m_channelID(channelID)
{
}

ThorQ::Networking::Message::~Message()
{
}

ENetPacket* ThorQ::Networking::Message::packet() const
{
    return m_packet.get();
}

std::uint8_t ThorQ::Networking::Message::channelID() const
{
    return m_channelID;
}

ThorQ::Networking::Message& ThorQ::Networking::Message::operator=(const ThorQ::Networking::Message& other)
{
    m_packet = other.m_packet;
    m_channelID = other.m_channelID;
    return *this;
}
