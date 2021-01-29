#ifndef CONNECTION_H
#define CONNECTION_H

#include "message.h"

#include "typedefs_global.h"

#include "asio_common.h"
#include "concurrentqueue.h"

#include <shared_mutex>
#include <atomic>
#include <span>
#include <memory>
#include <cstdint>

namespace ThorQ {
namespace Networking {
class Connection : public std::enable_shared_from_this<Connection>
{
protected:
    Connection();
    Connection(asio::io_context& asio);
public:
    virtual ~Connection();

    bool isTcp() const;
    bool isUdp() const;

    virtual void disconnect() = 0;

    inline std::uint64_t totalDataSent() const { return m_totalSentData; }
    inline std::uint64_t totalPacketsSent() const { return m_totalSentPackets; }
    inline std::uint64_t totalDataReceived() const { return m_totalReceivedData; }
    inline std::uint64_t totalPacketsReceived() const { return m_totalReceivedPackets; }

    std::shared_ptr<ThorQ::Networking::ConnectionHandlerInterface> connectionHandler() const;
    void setConnectionHandler(std::shared_ptr<ThorQ::Networking::ConnectionHandlerInterface> handler);

    virtual void send(std::shared_ptr<std::vector<std::uint8_t>> data) = 0;
protected:
    virtual void writeHeader(ThorQ::Networking::Message msg) = 0;
    virtual void writeBody(ThorQ::Networking::Message msg) = 0;
    virtual void writeEnd(){}

    virtual void readHeader(){}
    virtual void readBody(ThorQ::Networking::IncomingMessage msg) = 0;
    virtual void readEnd(ThorQ::Networking::IncomingMessage msg) = 0;

    virtual void asioClose() = 0;

    asio::io_context& m_asio;

    moodycamel::ConcurrentQueue<ThorQ::Networking::Message> m_messageQueue;
    moodycamel::ConsumerToken m_messageQueueToken;

    std::atomic_uint64_t m_totalSentData;
    std::atomic_uint64_t m_totalSentPackets;
    std::atomic_uint64_t m_totalReceivedData;
    std::atomic_uint64_t m_totalReceivedPackets;
private:
    std::shared_mutex l_connectionHandler;
    std::shared_ptr<ThorQ::Networking::ConnectionHandlerInterface> m_connectionHandler;
};
}
}

#endif // CONNECTION_H
