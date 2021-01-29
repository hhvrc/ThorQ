#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include "typedefs_global.h"

#include <vector>
#include <memory>
#include <cstdint>

namespace ThorQ {
namespace Networking {
class ConnectionHandlerInterface
{
public:
    virtual ~ConnectionHandlerInterface(){}

    std::shared_ptr<ThorQ::Networking::Connection> getConnection();
protected:
    friend ThorQ::Networking::Tcp::Connection;
    friend ThorQ::Networking::Udp::Connection;
    virtual void onConnect() = 0;
    virtual void onDisconnect() = 0;
    virtual bool onHeader(std::shared_ptr<ThorQ::Networking::MessageHeader> header) = 0;
    virtual void onMessage(std::shared_ptr<std::vector<std::uint8_t>> message) = 0;
private:
    std::weak_ptr<ThorQ::Networking::Connection> m_connection;
};
}
}

#endif // CONNECTIONHANDLER_H
