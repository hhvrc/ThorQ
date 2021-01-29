#include "apiclient.h"

#include "apiconnectionhandler.h"
#include <networking/tcpconnection.h>

#include <fmt/core.h>

ThorQ::ApiClient::ApiClient()
    : ThorQ::Networking::Tcp::Client()
{
    fmt::print("[CLIENT] Constructed\n");
}

ThorQ::ApiClient::~ApiClient()
{
    fmt::print("[CLIENT] Destroyed\n");
}

bool ThorQ::ApiClient::onConnect(std::shared_ptr<ThorQ::Networking::Tcp::Connection> connection)
{
    fmt::print("[CLIENT] Connected\n");
    connection->setConnectionHandler(std::make_shared<ThorQ::ApiConnectionHandler>());
    return true;
}

void ThorQ::ApiClient::onDisconnect(std::shared_ptr<ThorQ::Networking::Tcp::Connection> connection)
{
    fmt::print("[CLIENT] Disconnected\n");
}
