#ifndef UDPCONNECTION_H
#define UDPCONNECTION_H

#include "enums.h"
#include "encoding.h"
#include "typedefs_global.h"

#include "asio_common.h"

#include <vector>
#include <memory>
#include <atomic>
#include <cstdint>

namespace ThorQ {
namespace Networking {
class UdpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    UdpConnection(asio::io_context& asio, asio::ip::udp::socket socket);
    ~UdpConnection();

    void connect(const asio::ip::udp::resolver::results_type& endpoints);
    void disconnect();

    bool isOpen() const;

    inline std::uint64_t totalDataSent() const { return m_totalSentData.load(std::memory_order::relaxed); }
    inline std::uint64_t totalPacketsSent() const { return m_totalSentPackets.load(std::memory_order::relaxed); }
    inline std::uint64_t totalDataReceived() const { return m_totalReceivedData.load(std::memory_order::relaxed); }
    inline std::uint64_t totalPacketsReceived() const { return m_totalReceivedPackets.load(std::memory_order::relaxed); }

    void messageSend(std::shared_ptr<std::vector<std::uint8_t>> message);
protected:
    virtual void onConnect(std::vector<std::uint8_t> address, std::uint16_t port) = 0;
    virtual void onDisconnect() = 0;
    virtual bool onHeader(const ThorQ::Encoding::MessageHeader* header) = 0;
    virtual void onMessage(std::shared_ptr<std::vector<std::uint8_t>> message) = 0;
private:
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

    asio::ip::udp::socket m_socket;

    std::atomic<ConnectionStatus> m_status;
    std::atomic_uint64_t m_totalSentData;
    std::atomic_uint64_t m_totalSentPackets;
    std::atomic_uint64_t m_totalReceivedData;
    std::atomic_uint64_t m_totalReceivedPackets;
};
}
}

#endif // UDPCONNECTION_H
