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

void ThorQ::ApiClient::onCreatedConnection(std::shared_ptr<ThorQ::Networking::Tcp::Connection> connection)
{
    connection->setConnectionHandler(std::make_shared<ThorQ::ApiConnectionHandler>());
}
