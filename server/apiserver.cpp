#include "apiserver.h"

#include "apiconnectionhandler.h"
#include <networking/tcpconnection.h>

ThorQ::ApiServer::ApiServer(uint16_t port)
    : ThorQ::Networking::Tcp::Server(port)
{

}

ThorQ::ApiServer::~ApiServer()
{
}

bool ThorQ::ApiServer::onConnect(std::shared_ptr<ThorQ::Networking::Tcp::Connection> connection)
{
    connection->setConnectionHandler(std::shared_ptr<ThorQ::Networking::ConnectionHandlerInterface>(new ThorQ::ApiConnectionHandler()));

    std::string str = "Hello client!";
    connection->send(std::make_shared<std::vector<std::uint8_t>>(str.begin(), str.end()));
    return true;
}

void ThorQ::ApiServer::onDisconnect(std::shared_ptr<ThorQ::Networking::Tcp::Connection> connection)
{

}
