#ifndef APICLIENTCONNECTION_H
#define APICLIENTCONNECTION_H

#include "typedefs_client.h"

#include <networking/tcpconnection.h>
#include <cryptography/encryption.h>
#include <cryptography/signer.h>
#include <concurrentqueue.h>
#include <typedefs_global.h>
#include <constants.h>

#include <span>
#include <array>
#include <vector>
#include <memory>
#include <shared_mutex>
#include <cstdint>

namespace ThorQ {
class ApiClientConnection final : public ThorQ::Networking::TcpConnection
{
public:
    ApiClientConnection(asio::io_context& asio, asio::ip::tcp::socket socket,
                     moodycamel::ConcurrentQueue<std::shared_ptr<std::vector<std::uint8_t>>>& incomingQueue,
                     moodycamel::ConcurrentQueue<std::shared_ptr<std::vector<std::uint8_t>>>& outgoingQueue
                     );
    ApiClientConnection(ApiClientConnection&&) = default;

    ~ApiClientConnection();
private:
    // Event handlers
    void onError(std::error_code ec) override;
    void onConnect(std::vector<std::uint8_t> address, std::uint16_t port) override;
    void onDisconnect() override;
    bool onHeader(const ThorQ::Encoding::MessageHeader* header) override;
    void onMessage(std::shared_ptr<std::vector<std::uint8_t>> message) override;

    moodycamel::ConcurrentQueue<std::shared_ptr<std::vector<std::uint8_t>>>& m_incomingMessages;
    moodycamel::ProducerToken m_incomingMessagesToken;

    moodycamel::ConcurrentQueue<std::shared_ptr<std::vector<std::uint8_t>>>& m_outgoingMessages;
    moodycamel::ConsumerToken m_outgoingMessagesToken;
};
}

#endif // APICLIENTCONNECTION_H
