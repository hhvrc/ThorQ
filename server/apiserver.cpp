#include "apiserver.h"

#include <networking/tcpconnection.h>
#include "constants.h"

#include <fmt/core.h>

ThorQ::ApiServer::ApiServer(std::uint16_t port)
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
}

ThorQ::ApiServer::~ApiServer()
{
    stop();
}

bool ThorQ::ApiServer::start(unsigned int nproc)
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

void ThorQ::ApiServer::stop()
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



void ThorQ::ApiServer::messageSend(std::shared_ptr<ThorQ::ApiServerConnection> client, std::shared_ptr<std::vector<std::uint8_t>> message)
{
    if (client != nullptr && client->status() == ConnectionStatus::Connected)
    {
        client->messageSend(std::move(message));
    }
    else
    {
        cleanupConnections();
    }
}

void ThorQ::ApiServer::messageBroadcast(std::shared_ptr<std::vector<std::uint8_t>> data, std::shared_ptr<ThorQ::ApiServerConnection> ignore)
{
    bool invalidClientExists = false;

    auto ignorePtr = ignore.get();

    for (auto& clientWeak : m_connections)
    {
        // Try to lock the weak pointer
        auto client = clientWeak.lock();

        // Get the underlying pointer for more efficient access
        auto clientPtr = client.get();

        if (clientPtr != nullptr && clientPtr->status() == ConnectionStatus::Connected)
        {
            if (clientPtr != ignorePtr)
            {
                clientPtr->messageSend(data);
            }
        }
        else
        {
            invalidClientExists = true;
        }
    }

    if (invalidClientExists)
    {
        cleanupConnections();
    }
}

bool ThorQ::ApiServer::onConnect(std::shared_ptr<ThorQ::ApiServerConnection> connection)
{
    //std::string str = "Hello client!";
    //connection->messageSend(std::make_shared<std::vector<std::uint8_t>>(str.begin(), str.end()));
    return true;
}


void ThorQ::ApiServer::reset()
{
    stop();
    m_asio.reset();
}

void ThorQ::ApiServer::waitForClientConnection()
{
    m_asioAcceptor.async_accept(std::bind(&ApiServer::acceptCompletionHandler, this, std::placeholders::_1, std::placeholders::_2));
}

// https://stackoverflow.com/questions/45507041/how-to-check-if-weak-ptr-is-empty-non-assigned
template <typename T>
constexpr bool is_uninitialized(const std::weak_ptr<T>& weak) {
    using wt = std::weak_ptr<T>;
    return !weak.owner_before(wt{}) && !wt{}.owner_before(weak);
}

void ThorQ::ApiServer::cleanupConnections() {
    m_connections.erase(std::remove_if(m_connections.begin(), m_connections.end(), is_uninitialized<ThorQ::ApiServerConnection>), m_connections.end());
}

void ThorQ::ApiServer::acceptCompletionHandler(const std::error_code &ec, asio::ip::tcp::socket socket)
{
    if (!ec) {
        fmt::print("[SERVER] New connection: {}:{}\n", socket.remote_endpoint().address().to_string(), socket.remote_endpoint().port());

        auto connection = std::make_shared<ThorQ::ApiServerConnection>(m_asio, std::move(socket));

        if (onConnect(connection))
        {
            connection->accept();

            {
                std::unique_lock l(l_connections);
                m_connections.push_back(std::move(connection));
            }
        }
        else
        {
            fmt::print("[SERVER] Connection yeeted\n");
        }
    }
    else {
        fmt::print(stderr, "[SERVER] Connection error: {}\n", ec.message());
    }

    waitForClientConnection();
}
