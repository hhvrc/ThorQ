#ifndef APISERVER_H
#define APISERVER_H

#include "encoding.h"
#include "apiserver_connection.h"
#include "enums.h"
#include "typedefs_global.h"

#include "flatbuffers/flatbuffers.h"
#include "concurrentqueue.h"
#include "asio_common.h"

#include <shared_mutex>
#include <list>
#include <deque>
#include <span>
#include <cstdint>

namespace ThorQ {
class ApiServer final
{
public:
    ApiServer(std::uint16_t port);
    ~ApiServer();

    bool start(unsigned int nproc = std::thread::hardware_concurrency());
    void stop();

    inline ProcessStatus status() const noexcept { return m_status.load(std::memory_order::relaxed); }

    inline std::uint64_t totalDataSent() const noexcept { return m_totalSentData.load(std::memory_order::relaxed); }
    inline std::uint64_t totalPacketsSent() const noexcept { return m_totalSentPackets.load(std::memory_order::relaxed); }
    inline std::uint64_t totalDataReceived() const noexcept { return m_totalReceivedData.load(std::memory_order::relaxed); }
    inline std::uint64_t totalPacketsReceived() const noexcept { return m_totalReceivedPackets.load(std::memory_order::relaxed); }

    void messageSend(std::shared_ptr<ThorQ::ApiServerConnection> client, std::shared_ptr<std::vector<std::uint8_t>> message);
    void messageBroadcast(std::shared_ptr<std::vector<std::uint8_t>> data, std::shared_ptr<ThorQ::ApiServerConnection> ignore);
private:
    bool onConnect(std::shared_ptr<ThorQ::ApiServerConnection> connection);

    void reset();
    void waitForClientConnection();
    void cleanupConnections();

    void acceptCompletionHandler(const std::error_code& ec, asio::ip::tcp::socket socket);

    asio::io_context m_asio;
    asio::ip::tcp::acceptor m_asioAcceptor;

    std::vector<std::thread> m_threads;

    std::atomic<ProcessStatus> m_status;

    std::atomic_uint64_t m_totalSentData;
    std::atomic_uint64_t m_totalSentPackets;
    std::atomic_uint64_t m_totalReceivedData;
    std::atomic_uint64_t m_totalReceivedPackets;

    std::shared_mutex l_connections;
    std::deque<std::weak_ptr<ThorQ::ApiServerConnection>> m_connections;
};
}

#endif // APISERVER_H
