#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "enums.h"
#include "encoding.h"
#include "typedefs_global.h"

#include "asio_common.h"

#include <shared_mutex>
#include <vector>
#include <memory>
#include <atomic>
#include <cstdint>

namespace ThorQ::Networking {
class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
    TcpConnection() = delete;
    TcpConnection(const TcpConnection&) = delete;
    TcpConnection& operator=(const TcpConnection&) = delete;
public:
    TcpConnection(asio::io_context& asio, asio::ip::tcp::socket socket);
    TcpConnection(TcpConnection&& other);
    ~TcpConnection();

    void accept();
    void connect(const asio::ip::tcp::resolver::results_type& endpoints);
    void disconnect();

    inline ConnectionStatus status() const noexcept { return m_status.load(std::memory_order::relaxed); }

    std::error_code latestErrorCode() const;

    inline std::uint64_t totalDataSent() const noexcept { return m_totalSentData.load(std::memory_order::relaxed); }
    inline std::uint64_t totalPacketsSent() const noexcept { return m_totalSentPackets.load(std::memory_order::relaxed); }
    inline std::uint64_t totalDataReceived() const noexcept { return m_totalReceivedData.load(std::memory_order::relaxed); }
    inline std::uint64_t totalPacketsReceived() const noexcept { return m_totalReceivedPackets.load(std::memory_order::relaxed); }

    void messageSend(std::shared_ptr<std::vector<std::uint8_t>> message);
protected:
    virtual void onError(std::error_code ec) = 0;
    virtual void onConnect(std::vector<std::uint8_t> address, std::uint16_t port) = 0;
    virtual void onDisconnect() = 0;
    virtual bool onHeader(const ThorQ::Encoding::MessageHeader* header) = 0;
    virtual void onMessage(std::shared_ptr<std::vector<std::uint8_t>> message) = 0;
private:
    inline void setStatus(ConnectionStatus status) noexcept { m_status.store(status, std::memory_order_relaxed); }
    inline bool setStatusIf(ConnectionStatus& expected, ConnectionStatus newStatus) { return m_status.compare_exchange_strong(expected, newStatus, std::memory_order::relaxed, std::memory_order::acquire); }

    void setErrorCode(const std::error_code& ec);

    void writeDone();

    void readHeader();
    void readBody(std::shared_ptr<std::vector<std::uint8_t>> msg);
    void readDone(std::shared_ptr<std::vector<std::uint8_t>> msg);

    void asioClose();

    void handleErrorCode(const std::error_code& ec);

    void connectCompletionHandler(const std::error_code& ec, const asio::ip::tcp::endpoint& endpoint);
    void writeMessageCompletionHandler(const std::error_code& ec, const std::size_t length, std::shared_ptr<std::vector<std::uint8_t>> message);
    void readHeaderCompletionHandler(const std::error_code& ec, const std::size_t length, std::shared_ptr<std::vector<std::uint8_t>> message);
    void readBodyCompletionHandler(const std::error_code& ec, const std::size_t length, std::shared_ptr<std::vector<std::uint8_t>> message);

    asio::io_context& m_asio;

    asio::ip::tcp::socket m_socket;

    std::atomic<ConnectionStatus> m_status;

    std::shared_mutex l_errorCode;
    std::error_code m_errorCode;

    std::atomic_uint64_t m_totalSentData;
    std::atomic_uint64_t m_totalSentPackets;
    std::atomic_uint64_t m_totalReceivedData;
    std::atomic_uint64_t m_totalReceivedPackets;
};
}

#endif // TCPCONNECTION_H
