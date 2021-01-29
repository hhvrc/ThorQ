#include "abstractconnection.h"
#include "tcpconnection.h"
#include "udpconnection.h"

ThorQ::Networking::Connection::Connection(asio::io_context& asio)
    : m_asio(asio)
    , m_messageQueue()
    , m_messageQueueToken(m_messageQueue)
    , m_totalSentData(0)
    , m_totalSentPackets(0)
    , m_totalReceivedData(0)
    , m_totalReceivedPackets(0)
    , l_connectionHandler()
    , m_connectionHandler(nullptr)
{
}

ThorQ::Networking::Connection::~Connection()
{
}

bool ThorQ::Networking::Connection::isTcp() const
{
    return typeid(*this) == typeid(ThorQ::Networking::Tcp::Connection);
}

bool ThorQ::Networking::Connection::isUdp() const
{
    return false;//typeid(*this) == typeid(ThorQ::Networking::Udp::Connection);
}

std::shared_ptr<ThorQ::Networking::ConnectionHandlerInterface> ThorQ::Networking::Connection::connectionHandler() const
{
    std::unique_lock l(const_cast<std::shared_mutex&>(l_connectionHandler));
    return m_connectionHandler;
}

void ThorQ::Networking::Connection::setConnectionHandler(std::shared_ptr<ThorQ::Networking::ConnectionHandlerInterface> handler)
{
    std::unique_lock l(const_cast<std::shared_mutex&>(l_connectionHandler));
    m_connectionHandler = handler;
}
