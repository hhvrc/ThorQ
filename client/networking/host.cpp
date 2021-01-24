#include "host.h"

#include <array>
#include <string>

#include <enet.h>

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

#include "connectionhandler.h"

#define VER_STRING(MAJOR, MINOR, PATCH) #MAJOR "." #MINOR "." #PATCH
const char* ThorQ::Networking::Host::Version()
{
    return "ENet-" VER_STRING(ENET_VERSION_MAJOR, ENET_VERSION_MINOR, ENET_VERSION_PATCH);
}

std::atomic_bool g_initialized = false;
bool ThorQ::Networking::Host::Initialize()
{
    if (!g_initialized)
    {
        g_initialized = (enet_initialize() == 0);
    }

    return g_initialized;
}
void ThorQ::Networking::Host::DeInitialize()
{
    if (g_initialized)
    {
        g_initialized = false;
        enet_deinitialize();
    }
}

ThorQ::Networking::Host* ThorQ::Networking::Host::CreateHost(std::uint8_t channelLimit, QObject* parent)
{
    if (ThorQ::Networking::Host::Initialize())
    {
        ENetHost* host = enet_host_create(nullptr, 1, channelLimit, 0, 0);

        if (host != nullptr)
        {
            return new ThorQ::Networking::Host(host, parent);
        }
    }

    return nullptr;
}

ThorQ::Networking::Host* ThorQ::Networking::Host::CreateHost(QString hostname, std::uint16_t port, std::size_t peerLimit, std::uint8_t channelLimit, QObject *parent)
{
    if (ThorQ::Networking::Host::Initialize())
    {
        ENetAddress addr;

        if (enet_address_set_host_new(&addr, hostname.toStdString().c_str()) == 0)
        {
            addr.port = port;

            ENetHost* host = enet_host_create(&addr, peerLimit, channelLimit, 0, 0);


            if (host != nullptr)
            {
                return new ThorQ::Networking::Host(host, parent);
            }
        }
    }

    return nullptr;
}

ThorQ::Networking::Host::Host(ENetHost* host, QObject* parent)
    : QObject(parent)
    , m_serviceTimer(this)
    , m_statisticsTimer(this)
    , m_host(host)
{
    // Service timer
    QObject::connect(&m_serviceTimer, &QTimer::timeout, this, &ThorQ::Networking::Host::service);
    m_serviceTimer.setSingleShot(false);
    m_serviceTimer.setInterval(1);
    m_serviceTimer.start();

    // Statistics timer
    QObject::connect(&m_statisticsTimer, &QTimer::timeout, this, &ThorQ::Networking::Host::updateStats);
    m_statisticsTimer.setSingleShot(false);
    m_statisticsTimer.setInterval(250);
    m_statisticsTimer.start();
}

ThorQ::Networking::Host::~Host()
{
    m_statisticsTimer.stop();
    m_serviceTimer.stop();

    if (m_host != nullptr)
    {
        enet_host_destroy(m_host);
    }
}

std::uint64_t ThorQ::Networking::Host::txData() const
{
    return m_totalDataTx;
}

std::uint32_t ThorQ::Networking::Host::txSpeed() const
{
    return m_speedTx;
}

std::uint64_t ThorQ::Networking::Host::rxData() const
{
    return m_totalDataRx;
}

std::uint32_t ThorQ::Networking::Host::rxSpeed() const
{
    return m_speedRx;
}

void ThorQ::Networking::Host::connect(QString hostname, std::uint16_t port, std::uint8_t channelCount, ThorQ::Networking::ConnectionHandler* handler)
{
    if (handler == nullptr) {
        return;
    }

    qDebug() << "Connecting to" << hostname;

    ENetAddress addr;

    if (enet_address_set_host_new(&addr, hostname.toStdString().c_str()) != 0)
    {
        emit error("Failed to resolve hostname \"" + hostname + "\"");
        return;
    }

    addr.port = port;

    ENetPeer* peer = enet_host_connect(m_host, &addr, channelCount, 0);
    if (peer != nullptr)
    {
        handler->setPeer(peer);
        handler->setHost(this);
    }
}

void ThorQ::Networking::Host::service()
{
    ENetEvent event;
    while (enet_host_service(m_host, &event, 0) > 0)
    {
        if (event.peer == nullptr) {
            continue;
        }

        auto handler = reinterpret_cast<ThorQ::Networking::ConnectionHandler*>(event.peer->data);

        if (event.type == ENET_EVENT_TYPE_CONNECT)
        {
            if (handler == nullptr)
            {
                handler = new ThorQ::Networking::ConnectionHandler(this);
                handler->setHost(this);

                QObject::connect(&m_statisticsTimer, &QTimer::timeout, handler, &ThorQ::Networking::ConnectionHandler::updateStats);
                emit connectionIncoming(handler);
            }
            else
            {
                QObject::connect(&m_statisticsTimer, &QTimer::timeout, handler, &ThorQ::Networking::ConnectionHandler::updateStats);
                QMetaObject::invokeMethod(handler, "handleEvent", Qt::QueuedConnection, Q_ARG(ENetEvent, event));
            }
        }
        else if (handler != nullptr)
        {
            if (event.type == ENET_EVENT_TYPE_DISCONNECT_TIMEOUT || event.type == ENET_EVENT_TYPE_DISCONNECT)
            {
                handler->clear();
            }
            QMetaObject::invokeMethod(handler, "handleEvent", Qt::QueuedConnection, Q_ARG(ENetEvent, event));
        }

    }
}

void ThorQ::Networking::Host::updateStats()
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

void ThorQ::Networking::Host::sendUdp(ENetPeer* peer, ThorQ::Networking::Message message)
{
    if (peer == nullptr) {
        return;
    }

    // Make copy of packet
    ENetPacket* packet = enet_packet_copy(message.packet());

    if (enet_peer_send(peer, message.channelID(), packet) == -1) {
        // ERROR
        return;
    }
}

void ThorQ::Networking::Host::disconnect(ENetPeer* peer, std::uint32_t reason)
{
    enet_peer_disconnect(peer, reason);
}

void ThorQ::Networking::Host::disconnectNow(ENetPeer* peer, std::uint32_t reason)
{
    enet_peer_disconnect_now(peer, reason);
}

void ThorQ::Networking::Host::disconnectLater(ENetPeer* peer, std::uint32_t reason)
{
    enet_peer_disconnect_later(peer, reason);
}
