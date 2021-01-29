#ifndef ICLIENT_H
#define ICLIENT_H

#include "enums.h"
#include "typedefs_global.h"

#include "asio_common.h"

#include <string>
#include <deque>
#include <span>
#include <thread>
#include <memory>
#include <atomic>
#include <cstdint>

namespace ThorQ {
namespace Networking {
namespace Tcp {
class Client
{
public:
    Client();
    ~Client();

    bool connect(const std::string& host, std::uint16_t port);
    void disconnect();

    bool isConnected() const;

    std::uint64_t totalDataSent() const { return m_totalSentData; }
    std::uint64_t totalPacketsSent() const { return m_totalSentPackets; }
    std::uint64_t totalDataReceived() const { return m_totalReceivedData; }
    std::uint64_t totalPacketsReceived() const { return m_totalReceivedPackets; }
protected:
    virtual bool onConnect(std::shared_ptr<ThorQ::Networking::Tcp::Connection> connection) = 0;
    virtual void onDisconnect(std::shared_ptr<ThorQ::Networking::Tcp::Connection> connection) = 0;
private:
    asio::io_context m_asio;

    std::thread m_thread;

    std::atomic<ConnectionStatus> m_status;

    std::atomic_uint64_t m_totalSentData;
    std::atomic_uint64_t m_totalSentPackets;
    std::atomic_uint64_t m_totalReceivedData;
    std::atomic_uint64_t m_totalReceivedPackets;

    std::shared_ptr<ThorQ::Networking::Tcp::Connection> m_connection;
};
}
}
}

#endif // ICLIENT_H
