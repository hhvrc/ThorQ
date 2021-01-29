#ifndef SERVER_H
#define SERVER_H

#include "message.h"
#include "enums.h"
#include "typedefs_global.h"

#include "flatbuffers/flatbuffers.h"
#include "concurrentqueue.h"
#include "asio_common.h"

#include <shared_mutex>
#include <deque>
#include <span>
#include <cstdint>

namespace ThorQ {
namespace Networking {
namespace Tcp {
class Server
{
public:
    Server(std::uint16_t port);
    ~Server();

    bool start(unsigned int nproc = std::thread::hardware_concurrency());
    void stop();

    ProcessStatus status() const { return m_status; }

    std::uint64_t totalDataSent() const { return m_totalSentData; }
    std::uint64_t totalPacketsSent() const { return m_totalSentPackets; }
    std::uint64_t totalDataReceived() const { return m_totalReceivedData; }
    std::uint64_t totalPacketsReceived() const { return m_totalReceivedPackets; }

    void messageSend(std::shared_ptr<ThorQ::Networking::Tcp::Connection> client, std::shared_ptr<std::vector<std::uint8_t>> payload);
    void messageBroadcast(std::shared_ptr<std::vector<std::uint8_t>> data, std::shared_ptr<ThorQ::Networking::Tcp::Connection> ignore);
protected:
    virtual bool onConnect(std::shared_ptr<ThorQ::Networking::Tcp::Connection> connection) = 0;
    virtual void onDisconnect(std::shared_ptr<ThorQ::Networking::Tcp::Connection> connection) = 0;
private:
    void reset();
    void waitForClientConnection();

    asio::io_context m_asio;
    asio::ip::tcp::acceptor m_asioAcceptor;

    std::vector<std::thread> m_threads;

    std::atomic<ProcessStatus> m_status;

    std::atomic_uint64_t m_totalSentData;
    std::atomic_uint64_t m_totalSentPackets;
    std::atomic_uint64_t m_totalReceivedData;
    std::atomic_uint64_t m_totalReceivedPackets;

    std::shared_mutex l_connections;
    std::deque<std::shared_ptr<ThorQ::Networking::Tcp::Connection>> m_connections;
};
} // Tcp
} // Networking
} // ThorQ

#endif // SERVER_H
