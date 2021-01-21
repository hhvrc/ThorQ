#include "connectionhandler.h"

#include <enums.h>

#include "host.h"


ThorQ::Networking::ConnectionHandler::ConnectionHandler(QObject* parent)
    : QObject(parent)
    , m_peer(nullptr)
    , m_rtt(0)
    , m_dataTx(0)
    , m_dataRx(0)
    , m_speedTx(0)
    , m_speedRx(0)
{
}

uint32_t ThorQ::Networking::ConnectionHandler::rtt() const
{
    return m_rtt;
}

uint64_t ThorQ::Networking::ConnectionHandler::txData() const
{
    return m_dataTx;
}

uint64_t ThorQ::Networking::ConnectionHandler::rxData() const
{
    return m_dataRx;
}

uint32_t ThorQ::Networking::ConnectionHandler::txSpeed() const
{
    return m_speedTx;
}

uint32_t ThorQ::Networking::ConnectionHandler::rxSpeed() const
{
    return m_speedRx;
}

void ThorQ::Networking::ConnectionHandler::sendUdp(ThorQ::Networking::Message message)
{
    QMetaObject::invokeMethod(m_host, "sendUdp", Qt::QueuedConnection, Q_ARG(ENetPeer*, m_peer), Q_ARG(ThorQ::Networking::Message, message));
}

void ThorQ::Networking::ConnectionHandler::disconnect(std::uint32_t reason)
{
    QMetaObject::invokeMethod(m_host, "disconnect", Qt::QueuedConnection, Q_ARG(ENetPeer*, m_peer), Q_ARG(std::uint32_t, reason));
}

void ThorQ::Networking::ConnectionHandler::disconnectNow(std::uint32_t reason)
{
    QMetaObject::invokeMethod(m_host, "disconnectNow", Qt::QueuedConnection, Q_ARG(ENetPeer*, m_peer), Q_ARG(std::uint32_t, reason));
}

void ThorQ::Networking::ConnectionHandler::disconnectLater(std::uint32_t reason)
{
    QMetaObject::invokeMethod(m_host, "disconnectLater", Qt::QueuedConnection, Q_ARG(ENetPeer*, m_peer), Q_ARG(std::uint32_t, reason));
}


void ThorQ::Networking::ConnectionHandler::clear()
{
    if (m_peer != nullptr) {
        m_peer->data = nullptr;
        m_peer = nullptr;
    }

    m_rtt = 0;
    m_dataTx = 0;
    m_dataRx = 0;
    m_speedTx = 0;
    m_speedRx = 0;
}

void ThorQ::Networking::ConnectionHandler::setHost(ThorQ::Networking::Host *host)
{
    m_host = host;
}

void ThorQ::Networking::ConnectionHandler::setPeer(ENetPeer* peer)
{
    clear();
    m_peer = peer;
}

void ThorQ::Networking::ConnectionHandler::updateStats()
{
    ENetPeer* peer = m_peer;

    if (peer != nullptr)
    {
        std::uint16_t newRtt = peer->roundTripTime;
        std::uint32_t incTxData = peer->totalDataSent;
        std::uint32_t incRxData = peer->totalDataReceived;
        std::uint32_t newTxSpeed = peer->outgoingBandwidth;
        std::uint32_t newRxSpeed = peer->incomingBandwidth;

        if (m_rtt != newRtt)
        {
            m_rtt = newRtt;
            emit rttChanged(newRtt);
        }

        if (incTxData != 0)
        {
            m_dataTx += incTxData;
            peer->totalDataSent = 0;
            emit txDataChanged(m_dataTx);
        }

        if (incRxData != 0)
        {
            m_dataRx += incRxData;
            peer->totalDataReceived = 0;
            emit rxDataChanged(m_dataRx);
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
}

void ThorQ::Networking::ConnectionHandler::handleEvent(ENetEvent event)
{
    switch (event.type) {
    case ENET_EVENT_TYPE_CONNECT:
        emit connected(event.data);
        break;
    case ENET_EVENT_TYPE_RECEIVE:
        emit udpReceived(Networking::Message(event));
        disconnect((std::uint32_t)THORQ_DISCONNECT_REASON::VERSION_INCOMPATIBLE);
        break;
    case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
        emit disconnected((std::uint32_t)THORQ_DISCONNECT_REASON::TIMED_OUT);
        break;
    case ENET_EVENT_TYPE_DISCONNECT:
        emit disconnected(event.data);
        break;
    default:
        break;
    }

}
