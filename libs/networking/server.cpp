#include "server.h"

#include "tcpconnection.h"
#include "constants.h"

#include <fmt/core.h>

ThorQ::Networking::Tcp::Server::Server(std::uint16_t port)
    : m_asio()
    , m_asioAcceptor(m_asio, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), port))
    , m_threads()
    , m_status(ProcessStatus::Stopped)
    , m_totalSentData(0)
    , m_totalSentPackets(0)
    , m_totalReceivedData(0)
    , m_totalReceivedPackets(0)
    , l_connections()
    , m_connections()
{
    fmt::print("[SERVER] Created\n");
}

ThorQ::Networking::Tcp::Server::~Server()
{
    stop();

    fmt::print("[SERVER] Destroyed\n");
}

bool ThorQ::Networking::Tcp::Server::start(unsigned int nproc)
{
    ProcessStatus expected = ProcessStatus::Stopped;
    if (m_status.compare_exchange_strong(expected, ProcessStatus::Starting))
    {
        fmt::print("[SERVER] Starting with {} threads...\n", nproc);
        try
        {
            waitForClientConnection();

            for (unsigned int i = 0; i < nproc; i++)
            {
                m_threads.emplace_back([this](){ m_asio.run(); });
            }
        }
        catch (std::exception& ex)
        {
            fmt::print(stderr, "[SERVER] Failed to start: {}\n", ex.what());
            reset();
            return false;
        }

        fmt::print("[SERVER] Started\n");

        m_status.store(ProcessStatus::Running);

        return true;
    }


    return expected == ProcessStatus::Starting || expected == ProcessStatus::Running;
}

void ThorQ::Networking::Tcp::Server::stop()
{
    ProcessStatus expected = ProcessStatus::Running;
    if (m_status.compare_exchange_strong(expected, ProcessStatus::Stopping))
    {
        fmt::print("[SERVER] Stopping...\n");
        m_asio.stop();

        for (auto& thread : m_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        m_threads.clear();
        fmt::print("[SERVER] Stopped\n");

        m_status.store(ProcessStatus::Stopped);
    }
}

void ThorQ::Networking::Tcp::Server::messageSend(std::shared_ptr<ThorQ::Networking::Tcp::Connection> client, std::shared_ptr<std::vector<std::uint8_t>> data)
{
    if (client != nullptr && client->isConnected())
    {
        client->send(std::move(data));
    }
    else
    {
        onDisconnect(client);
        m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), client), m_connections.end());
        client.reset();

    }
}

void ThorQ::Networking::Tcp::Server::messageBroadcast(std::shared_ptr<std::vector<std::uint8_t>> data, std::shared_ptr<ThorQ::Networking::Tcp::Connection> ignore)
{
    bool invalidClientExists = false;

    for (auto& client : m_connections)
    {
        if (client != nullptr && client->isConnected())
        {
            if (client != ignore)
            {
                client->send(std::move(data));
            }
        }
        else
        {
            onDisconnect(client);
            client.reset();
            invalidClientExists = true;
        }
    }

    if (invalidClientExists)
    {
        m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), nullptr), m_connections.end());
    }
}

void ThorQ::Networking::Tcp::Server::reset()
{
    stop();
    m_asio.reset();
}

void ThorQ::Networking::Tcp::Server::waitForClientConnection()
{
    m_asioAcceptor.async_accept(
                [this](std::error_code ec, asio::ip::tcp::socket socket)
    {
        if (!ec) {
            fmt::print("[SERVER] New connection: {}:{}\n", socket.remote_endpoint().address().to_string(), socket.remote_endpoint().port());

            auto connection = std::make_shared<ThorQ::Networking::Tcp::Connection>(m_asio, std::move(socket));

            if (onConnect(connection))
            {
                connection->accept();

                {
                    std::unique_lock l(l_connections);
                    m_connections.push_back(std::move(connection));
                }

                fmt::print("[SERVER] Connection approved\n");
            }
            else
            {
                fmt::print("[SERVER] Connection yeeted\n");
            }
        }
        else {
            fmt::print("[SERVER] Connection error: {}\n", ec.message());
        }

        waitForClientConnection();
    });
}
