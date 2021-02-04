#include "apiclient_connection.h"

#include <cryptography/signer.h>
#include <systemid.h>
#include <encoding.h>
#include <enums.h>

#include <schemas_common.h>

#include <fmt/core.h>

#include <mutex>

ThorQ::ApiClientConnection::ApiClientConnection(asio::io_context& asio, asio::ip::tcp::socket socket,
                                          moodycamel::ConcurrentQueue<std::shared_ptr<std::vector<std::uint8_t>>>& incomingQueue,
                                          moodycamel::ConcurrentQueue<std::shared_ptr<std::vector<std::uint8_t>>>& outgoingQueue)
    : ThorQ::Networking::TcpConnection(asio, std::move(socket))
    , m_incomingMessages(incomingQueue)
    , m_incomingMessagesToken(m_incomingMessages)
    , m_outgoingMessages(outgoingQueue)
    , m_outgoingMessagesToken(m_outgoingMessages)
{
    fmt::print("[CONNECTION] Constructed\n");
}

ThorQ::ApiClientConnection::~ApiClientConnection()
{
    disconnect();
    fmt::print("[CONNECTION] Destroyed\n");
}

void ThorQ::ApiClientConnection::onError(std::error_code ec)
{
    fmt::print(stderr, "[CONNECTION] Error: {}\n", ec.message());
}

void ThorQ::ApiClientConnection::onConnect(std::vector<std::uint8_t> address, std::uint16_t port)
{
    fmt::print("[CONNECTION] Connected\n");
}

void ThorQ::ApiClientConnection::onDisconnect()
{
    fmt::print("[CONNECTION] Disconnected\n");
}

bool ThorQ::ApiClientConnection::onHeader(const ThorQ::Encoding::MessageHeader* header)
{
    fmt::print("[CONNECTION] Header\n");

    return header->bodySize >= ThorQ::Encoding::MinimumMessageSize && header->bodySize <= ThorQ::Encoding::MaximumMessageSize;
}

void ThorQ::ApiClientConnection::onMessage(std::shared_ptr<std::vector<std::uint8_t>> message)
{
    fmt::print("[CONNECTION] Message\n");

    while (!m_incomingMessages.enqueue(message)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        fmt::print(stderr, "[CONNECTION] Failed to queue message, retrying...\n");
    }
}
