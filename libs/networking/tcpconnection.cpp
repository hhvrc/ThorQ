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
    , m_status(other.status())
    , m_errorCode(std::move(other.m_errorCode))
    , m_totalSentData(other.totalDataSent())
    , m_totalSentPackets(other.totalPacketsSent())
    , m_totalReceivedData(other.totalDataReceived())
    , m_totalReceivedPackets(other.totalPacketsReceived())
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
    if (setStatusIf(expected, ConnectionStatus::Connected))
    {
        setStatus(ConnectionStatus::Connected);

        std::vector<std::uint8_t> address;
        address.reserve(16);

        auto endpoint = m_socket.remote_endpoint();

        if (endpoint.address().is_v4()) {
            auto asioAddr = endpoint.address().to_v4().to_bytes();
            address.insert(address.begin(), asioAddr.begin(), asioAddr.end());
        }
        else {
            auto asioAddr = endpoint.address().to_v6().to_bytes();
            address.insert(address.begin(), asioAddr.begin(), asioAddr.end());
        }

        onConnect(address, endpoint.port());
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

std::error_code ThorQ::Networking::TcpConnection::latestErrorCode() const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_errorCode));
    return m_errorCode;
}

void ThorQ::Networking::TcpConnection::messageSend(std::shared_ptr<std::vector<std::uint8_t>> message)
{
    fmt::print("[TCP-CONNECTION] messageSend {}\n", message->size());

    auto buffer = asio::buffer(message->data(), message->size());
    asio::async_write(m_socket, buffer, std::bind(&TcpConnection::writeMessageCompletionHandler, shared_from_this(), std::placeholders::_1, std::placeholders::_2, std::move(message)));
}

void ThorQ::Networking::TcpConnection::setErrorCode(const std::error_code& ec)
{
    std::unique_lock l(l_errorCode);
    m_errorCode = ec;
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
    auto buffer = asio::buffer(message->data() + ThorQ::Encoding::HeaderSize, message->size() - ThorQ::Encoding::HeaderSize);
    asio::async_read(m_socket, buffer, std::bind(&TcpConnection::readBodyCompletionHandler, shared_from_this(), std::placeholders::_1, std::placeholders::_2, std::move(message)));
}

void ThorQ::Networking::TcpConnection::readDone(std::shared_ptr<std::vector<std::uint8_t>> message)
{
    // Start next async read
    readHeader();

    // Process message
    onMessage(std::move(message));
}

void ThorQ::Networking::TcpConnection::asioClose()
{
    try {
        m_socket.close();
    } catch (...) {}
}

void ThorQ::Networking::TcpConnection::handleErrorCode(const std::error_code& ec)
{
    if (ec == std::errc::already_connected || ec == std::errc::connection_already_in_progress) {
        return; // This is not rly an error to care about
    }
    else if (ec == std::errc::connection_aborted) {
        // Do nothing, just disconnect without any fuzz
    }
    else if (ec == std::errc::timed_out) {
        fmt::print("[TCP-CONNECTION] Timeout!\n");
        onDisconnect();
    }
    else if (ec == std::errc::connection_reset) {
        fmt::print("[TCP-CONNECTION] Remote closed connection\n");
        onDisconnect();
    }
    else { // Includes std::errc::connection_refused
        fmt::print("[TCP-CONNECTION] Error: {} ({})\n", ec.message(), ec.value());
        setErrorCode(ec);
        setStatus(ConnectionStatus::Error);
        onError(std::move(ec));
        asioClose();
        return;
    }

    setStatus(ConnectionStatus::Disconnected);
    asioClose();
}

void ThorQ::Networking::TcpConnection::connectCompletionHandler(const std::error_code& ec, const asio::ip::tcp::endpoint& endpoint)
{
    if (ec) {
        handleErrorCode(ec);
        return;
    }

    setStatus(ConnectionStatus::Connected);

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

void ThorQ::Networking::TcpConnection::writeMessageCompletionHandler(const std::error_code& ec, std::size_t length, std::shared_ptr<std::vector<std::uint8_t>> message)
{
    if (ec) {
        handleErrorCode(ec);
        return;
    }

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

void ThorQ::Networking::TcpConnection::readHeaderCompletionHandler(const std::error_code& ec, std::size_t length, std::shared_ptr<std::vector<std::uint8_t>> message)
{
    if (ec) {
        handleErrorCode(ec);
        return;
    }

    if (length == 0) {
        readHeader();
    }

    if (length != ThorQ::Encoding::HeaderSize) {
        fmt::print("[TCP-CONNECTION] Only read {}/{} bytes\n", length, message->size());
        readHeader();
        return;
    }

    m_totalReceivedData += length;

    ThorQ::Encoding::MessageHeader* header = reinterpret_cast<ThorQ::Encoding::MessageHeader*>(message->data());

    if (onHeader(header))
    {
        std::uint32_t bodySize = ntohl(header->bodySize);

        if (bodySize > 0)
        {
            fmt::print("[TCP-CONNECTION] Reading body\n");
            message->resize(ThorQ::Encoding::HeaderSize + bodySize);
            readBody(std::move(message));
        }
        else
        {
            fmt::print("[TCP-CONNECTION] Empty header\n");
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

void ThorQ::Networking::TcpConnection::readBodyCompletionHandler(const std::error_code& ec, std::size_t length, std::shared_ptr<std::vector<std::uint8_t>> message)
{
    if (ec) {
        handleErrorCode(ec);
        return;
    }

    m_totalReceivedData += length;
    m_totalReceivedPackets++;
    readDone(std::move(message));
}
