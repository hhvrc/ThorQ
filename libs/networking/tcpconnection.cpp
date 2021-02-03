#include "tcpconnection.h"

#include "fmt/core.h"

ThorQ::Networking::TcpConnection::TcpConnection(asio::io_context& asio, asio::ip::tcp::socket socket)
    : m_asio(asio)
    , m_socket(std::move(socket))
    , m_status(ConnectionStatus::Disconnected)
    , m_totalSentData(0)
    , m_totalSentPackets(0)
    , m_totalReceivedData(0)
    , m_totalReceivedPackets(0)
{
    fmt::print("[TCP-CONNECTION] Constructed\n");
}

ThorQ::Networking::TcpConnection::TcpConnection(ThorQ::Networking::TcpConnection&& other)
    : m_asio(other.m_asio)
    , m_socket(std::move(other.m_socket))
    , m_status(other.m_status.load(std::memory_order::relaxed))
    , m_totalSentData(other.m_totalSentData.load(std::memory_order::relaxed))
    , m_totalSentPackets(other.m_totalSentPackets.load(std::memory_order::relaxed))
    , m_totalReceivedData(other.m_totalReceivedData.load(std::memory_order::relaxed))
    , m_totalReceivedPackets(other.m_totalReceivedPackets.load(std::memory_order::relaxed))
{
}

ThorQ::Networking::TcpConnection::~TcpConnection()
{
    fmt::print("[TCP-CONNECTION] Destructed\n");
}

void ThorQ::Networking::TcpConnection::accept()
{
    fmt::print("[TCP-CONNECTION] Accept\n");
    ConnectionStatus expected = ConnectionStatus::Disconnected;
    if (m_status.compare_exchange_strong(expected, ConnectionStatus::Connecting, std::memory_order::relaxed, std::memory_order::acquire))
    {
        readHeader();
    }
}

void ThorQ::Networking::TcpConnection::connect(const asio::ip::tcp::resolver::results_type& endpoints)
{
    fmt::print("[TCP-CONNECTION] Connect\n");

    asio::async_connect(m_socket, endpoints, std::bind(&TcpConnection::connectCompletionHandler, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void ThorQ::Networking::TcpConnection::disconnect()
{
    fmt::print("[TCP-CONNECTION] Disconnect\n");

    onDisconnect();
    asioClose();
}

void ThorQ::Networking::TcpConnection::messageSend(std::shared_ptr<std::vector<std::uint8_t>> message)
{
    fmt::print("[TCP-CONNECTION] messageSend\n");

    auto buffer = asio::buffer(message->data(), message->size());
    asio::async_write(m_socket, buffer, std::bind(&TcpConnection::writeMessageCompletionHandler, shared_from_this(), std::placeholders::_1, std::placeholders::_2, std::move(message)));
}

void ThorQ::Networking::TcpConnection::writeDone()
{
    fmt::print("[TCP-CONNECTION] WriteDone\n");
}

void ThorQ::Networking::TcpConnection::readHeader()
{
    fmt::print("[TCP-CONNECTION] ReadHeader\n");

    auto message = std::make_shared<std::vector<std::uint8_t>>();
    message->reserve(ThorQ::Encoding::TypicalMessageSize);
    message->resize(ThorQ::Encoding::HeaderSize);

    auto buffer = asio::buffer(message->data(), message->size());
    asio::async_read(m_socket, buffer, std::bind(&TcpConnection::readHeaderCompletionHandler, shared_from_this(), std::placeholders::_1, std::placeholders::_2, std::move(message)));
}

void ThorQ::Networking::TcpConnection::readBody(std::shared_ptr<std::vector<std::uint8_t>> message)
{
    auto buffer = asio::buffer(message->data(), message->size());
    asio::async_read(m_socket, buffer, std::bind(&TcpConnection::readBodyCompletionHandler, shared_from_this(), std::placeholders::_1, std::placeholders::_2, std::move(message)));
}

void ThorQ::Networking::TcpConnection::readDone(std::shared_ptr<std::vector<std::uint8_t>> message)
{
    onMessage(std::move(message));
    readHeader();
}

void ThorQ::Networking::TcpConnection::asioClose()
{
    try {
        m_socket.close();
    } catch (...) {}
}

void ThorQ::Networking::TcpConnection::handleErrorCode(const std::error_code &ec)
{
    if (ec.value() == 2) {
        fmt::print("[TCP-CONNECTION] Remote closed connection\n");
    }
    else {
        fmt::print(stderr, "[TCP-CONNECTION] Error reading header: {} ({})\n", ec.message(), ec.value());
    }
    disconnect();
}

void ThorQ::Networking::TcpConnection::connectCompletionHandler(const std::error_code& ec, const asio::ip::tcp::endpoint& endpoint)
{
    if (!ec)
    {
        m_status = ConnectionStatus::Connected;

        std::vector<std::uint8_t> address;
        address.reserve(16);

        if (endpoint.address().is_v4()) {
            auto asioAddr = endpoint.address().to_v4().to_bytes();
            address.insert(address.begin(), asioAddr.begin(), asioAddr.end());
        }
        else {
            auto asioAddr = endpoint.address().to_v6().to_bytes();
            address.insert(address.begin(), asioAddr.begin(), asioAddr.end());
        }

        onConnect(std::move(address), endpoint.port());

        readHeader();
    }
    else
    {
        fmt::print("[TCP-CONNECTION] Connect failed: {} ({})\n", ec.message(), ec.value());
        return;
    }
}

void ThorQ::Networking::TcpConnection::writeMessageCompletionHandler(const std::error_code& ec, std::size_t length, std::shared_ptr<std::vector<std::uint8_t>> message)
{
    if (!ec)
    {
        if (length == 0) {
            readHeader();
        }
        if (length != message->size()) {
            fmt::print("[TCP-CONNECTION] Only read {}/{} bytes\n", length, message->size());
        }

        m_totalSentData += length;
        m_totalSentPackets++;
        writeDone();
    }
    else
    {
        handleErrorCode(ec);
    }
}

void ThorQ::Networking::TcpConnection::readHeaderCompletionHandler(const std::error_code& ec, std::size_t length, std::shared_ptr<std::vector<std::uint8_t>> message)
{
    if (!ec)
    {
        if (length == 0) {
            readHeader();
        }
        if (length != ThorQ::Encoding::HeaderSize) {
            fmt::print("[TCP-CONNECTION] Only read {}/{} bytes\n", length, message->size());
        }

        m_totalReceivedData += length;

        auto header = reinterpret_cast<ThorQ::Encoding::MessageHeader*>(message->data());

        if (onHeader(header))
        {
            if (header->bodySize > 0)
            {
                message->resize(ThorQ::Encoding::HeaderSize + header->bodySize);

                readBody(std::move(message));
            }
            else
            {
                m_totalReceivedPackets++;
                // There is nothing more to do here
            }
        }
        else
        {
            fmt::print("[TCP-CONNECTION] Message rejected!\n");
            readHeader();
        }
    }
    else
    {
        handleErrorCode(ec);
    }
}

void ThorQ::Networking::TcpConnection::readBodyCompletionHandler(const std::error_code& ec, std::size_t length, std::shared_ptr<std::vector<std::uint8_t>> message)
{
    if (!ec)
    {
        m_totalReceivedData += length;
        m_totalReceivedPackets++;
        readDone(std::move(message));
    }
    else
    {
        handleErrorCode(ec);
    }
}
