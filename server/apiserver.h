#ifndef APISERVER_H
#define APISERVER_H

#include <networking/server.h>

#include "apiconnectionhandler.h"

namespace ThorQ {
class ApiServer final : public ThorQ::Networking::Tcp::Server
{
public:
    ApiServer(std::uint16_t port);
    ~ApiServer();
private:
    bool onConnect(std::shared_ptr<ThorQ::Networking::Tcp::Connection> connection) override;
    void onDisconnect(std::shared_ptr<ThorQ::Networking::Tcp::Connection> connection) override;
};
}

#endif // APISERVER_H
