#include "udpconnection.h"

#include <fmt/core.h>

ThorQ::Networking::UdpConnection::UdpConnection(asio::io_context& asio, asio::ip::udp::socket socket)
    : m_asio(asio)
    , m_socket(std::move(socket))
    , m_status(ConnectionStatus::Disconnected)
    , m_totalSentData(0)
    , m_totalSentPackets(0)
    , m_totalReceivedData(0)
    , m_totalReceivedPackets(0)
{
    fmt::print("[UDP-CONNECTION] Constructed\n");
}

ThorQ::Networking::UdpConnection::~UdpConnection()
{
    fmt::print("[UDP-CONNECTION] Destructed\n");
}
/*
void ThorQ::Networking::UdpConnection::connect(const asio::ip::udp::resolver::results_type& endpoints)
{
    fmt::print("[UDP-CONNECTION] Connect\n");

    asio::async_connect(m_socket, endpoints, std::bind(&UdpConnection::connectCompletionHandler, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void ThorQ::Networking::UdpConnection::disconnect()
{
    fmt::print("[UDP-CONNECTION] Disconnect\n");

    onDisconnect();
    asioClose();
}

void ThorQ::Networking::UdpConnection::messageSend(std::shared_ptr<std::vector<std::uint8_t>> message)
{
    fmt::print("[UDP-CONNECTION] messageSend\n");

    auto buffer = asio::buffer(message->data(), message->size());
    asio::async_write(m_socket, buffer, std::bind(&UdpConnection::writeMessageCompletionHandler, shared_from_this(), std::placeholders::_1, std::placeholders::_2, std::move(message)));
}

void ThorQ::Networking::UdpConnection::writeDone()
{
    fmt::print("[UDP-CONNECTION] WriteDone\n");
}

void ThorQ::Networking::UdpConnection::readHeader()
{
    fmt::print("[UDP-CONNECTION] ReadHeader\n");

    auto message = std::make_shared<std::vector<std::uint8_t>>();
    message->reserve(ThorQ::Encoding::TypicalMessageSize);
    message->resize(ThorQ::Encoding::HeaderSize);

    auto buffer = asio::buffer(message->data(), message->size());
    asio::async_read(m_socket, buffer, std::bind(&UdpConnection::writeMessageCompletionHandler, shared_from_this(), std::placeholders::_1, std::placeholders::_2, std::move(message)));
}

void ThorQ::Networking::UdpConnection::readBody(std::shared_ptr<std::vector<std::uint8_t>> message)
{
    auto buffer = asio::buffer(message->data(), message->size());
    asio::async_read(m_socket, buffer, std::bind(&UdpConnection::writeMessageCompletionHandler, shared_from_this(), std::placeholders::_1, std::placeholders::_2, std::move(message)));
}

void ThorQ::Networking::UdpConnection::readDone(std::shared_ptr<std::vector<std::uint8_t>> message)
{
    onMessage(std::move(message));
    readHeader();
}

void ThorQ::Networking::UdpConnection::asioClose()
{
    try {
        m_socket.close();
    } catch (...) {}
}

void ThorQ::Networking::UdpConnection::handleErrorCode(const std::error_code &ec)
{
    if (ec.value() == 2) {
        fmt::print("[UDP-CONNECTION] Remote closed connection\n");
    }
    else {
        fmt::print(stderr, "[UDP-CONNECTION] Error reading header: {} ({})\n", ec.message(), ec.value());
    }
    disconnect();
}
*/
