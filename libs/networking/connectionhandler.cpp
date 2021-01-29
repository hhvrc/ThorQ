#include "connectionhandler.h"

std::shared_ptr<ThorQ::Networking::Connection> ThorQ::Networking::ConnectionHandlerInterface::connection()
{
    std::shared_lock l(l_connection);
    return m_connection.lock();
}

void ThorQ::Networking::ConnectionHandlerInterface::setConnection(std::shared_ptr<ThorQ::Networking::Connection> connection)
{
    std::unique_lock l(l_connection);
    m_connection = connection;
}
