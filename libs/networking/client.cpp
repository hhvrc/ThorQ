#include "client.h"

#include "tcpconnection.h"

#include "fmt/core.h"

ThorQ::Networking::Tcp::Client::Client()
    : m_asio()
    , m_thread()
    , m_status(ConnectionStatus::Disconnected)
    , m_totalSentData()
    , m_totalSentPackets()
    , m_totalReceivedData()
    , m_totalReceivedPackets()
    , m_connection()
{
    fmt::print("[CLIENT] Created\n");
}

ThorQ::Networking::Tcp::Client::~Client()
{
    m_asio.stop();

    if (m_thread.joinable())
    {
        m_thread.join();
    }

    fmt::print("[CLIENT] Destroyed\n");
}

bool ThorQ::Networking::Tcp::Client::connect(const std::string& host, uint16_t port)
{
    ConnectionStatus expected = ConnectionStatus::Disconnected;
    if (m_status.compare_exchange_strong(expected, ConnectionStatus::Connecting))
    {
        try
        {
            auto connection = std::make_shared<ThorQ::Networking::Tcp::Connection>(m_asio, asio::ip::tcp::socket(m_asio));

            onCreatedConnection(connection);

            if (connection->connectionHandler() == nullptr) {
                return false;
            }

            m_connection = std::move(connection);

            asio::ip::tcp::resolver resolver(m_asio);
            auto endpoints = resolver.resolve(host, std::to_string(port));

            m_connection->connect(endpoints);

            m_thread = std::thread([this](){ m_asio.run(); });
        }
        catch (std::exception& ex)
        {
            fmt::print(stderr, "[CLIENT] Failed to connect: {}\n", ex.what());
            return false;
        }

        fmt::print("[SERVER] Connecting to {}[{}]...\n", host, port);
    }

    return false;
}

void ThorQ::Networking::Tcp::Client::disconnect()
{
    ConnectionStatus expected = ConnectionStatus::Connected;
    if (m_status.compare_exchange_strong(expected, ConnectionStatus::Disconnecting))
    {
        try
        {
            if (isConnected()) {
                m_connection->disconnect();
            }

            m_asio.stop();

            if (m_thread.joinable())
            {
                m_thread.join();
            }
        }
        catch (std::exception& ex)
        {
            fmt::print(stderr, "[CLIENT] Encountered error while stopping: {}\n", ex.what());
        }
    }
}

bool ThorQ::Networking::Tcp::Client::isConnected() const
{
    if (m_connection != nullptr)
    {
        return m_connection->isConnected();
    }

    return false;
}
