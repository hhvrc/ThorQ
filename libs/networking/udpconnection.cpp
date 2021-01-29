#include "udpconnection.h"

#include "hashing.h"
#include "utils.h"

#include "fmt/core.h"
/*
ThorQ::Networking::Udp::Connection::Connection(asio::io_context& asio, asio::ip::udp::socket socket)
    : ThorQ::Networking::Connection(asio)
    , l_socket()
    , m_socket(std::move(socket))
{

}

ThorQ::Networking::Udp::Connection::~Connection()
{
    disconnect();
}

void ThorQ::Networking::Udp::Connection::connect(const asio::ip::udp::resolver::results_type& endpoints)
{
    asio::async_connect(m_socket, endpoints,
                        [this](std::error_code ec, asio::ip::udp::endpoint endpoint)
    {
        THORQ_UNUSED(endpoint)
        if (!ec)
        {
            readHeader();
        }
        else
        {
            // TODO: ERROR "ec"
        }
    });
}

void ThorQ::Networking::Udp::Connection::disconnect()
{
    m_connectionHandler->onDisconnect();
    asio::post(m_asio, [this](){ asioClose(); });
}

bool ThorQ::Networking::Udp::Connection::isOpen() const
{
    std::scoped_lock l(const_cast<std::mutex&>(l_socket));
    return m_socket.is_open();
}

void ThorQ::Networking::Udp::Connection::send(std::shared_ptr<std::vector<uint8_t>> data)
{
    ThorQ::Networking::Message msg;

    msg.header = std::make_shared<ThorQ::Networking::MessageHeader>(data->size(), ThorQ::Hashing::Crc32(*data));
    msg.body = std::move(data);

    asio::post(m_asio, [this, msg](){ writeHeader(std::move(msg)); });
}

void ThorQ::Networking::Udp::Connection::writeHeader(ThorQ::Networking::Message msg)
{
    asio::async_write(m_socket, asio::buffer(msg.header.get(), sizeof(ThorQ::Networking::MessageHeader)),
                      [this, msg](std::error_code ec, std::size_t length)
    {
        if (!ec)
        {
            m_totalSentData += length;
            if (msg.header->size > 0)
            {
                writeBody(std::move(msg));
            }
            else
            {
                m_totalSentPackets++;
                writeEnd();
            }
        }
        else
        {
            // TODO: ERROR "ec"
            asioClose();
        }
    });
}

void ThorQ::Networking::Udp::Connection::writeBody(ThorQ::Networking::Message msg)
{
    asio::async_write(m_socket, asio::buffer(msg.body->data(), msg.body->size()),
                      [this, msg](std::error_code ec, std::size_t length)
    {
        if (!ec)
        {
            m_totalSentData += length;
            m_totalSentPackets++;
            writeEnd();
        }
        else
        {
            // TODO: ERROR "ec"
            asioClose();
        }
    });
}

void ThorQ::Networking::Udp::Connection::writeEnd()
{
    ThorQ::Networking::Message msg;
    if (m_messageQueue.try_dequeue(m_messageQueueToken, msg))
    {
        writeHeader(std::move(msg));
    }
}

void ThorQ::Networking::Udp::Connection::readHeader()
{
    IncomingMessage msg;
    msg.header = std::make_shared<ThorQ::Networking::MessageHeader>();

    asio::async_read(m_socket, asio::buffer(msg.header.get(), sizeof(ThorQ::Networking::MessageHeader)),
                     [this, msg](std::error_code ec, std::size_t length) mutable
    {
        if (!ec)
        {
            if (m_connectionHandler->onHeader(msg.header))
            {
                m_totalReceivedData += length;
                if (msg.header->size > 0)
                {
                    msg.body = std::make_shared<std::vector<std::uint8_t>>(msg.header->size);

                    readBody(std::move(msg));
                }
                else
                {
                    m_totalReceivedPackets++;
                    // There is nothing more to do here
                }
            }
            else
            {
                fmt::print("[CLIENT] Message rejected!\n");
                readHeader();
            }
        }
        else
        {
            // TODO: ERROR "ec"
            asioClose();
        }
    });
}

void ThorQ::Networking::Udp::Connection::readBody(ThorQ::Networking::IncomingMessage msg)
{
    asio::async_read(m_socket, asio::buffer(msg.body->data(), msg.body->size()),
                     [this, msg](std::error_code ec, std::size_t length)
    {
        if (!ec)
        {
            m_totalReceivedData += length;
            m_totalReceivedPackets++;
            readEnd(std::move(msg));
        }
        else
        {
            // TODO: ERROR "ec"
            asioClose();
        }
    });
}

void ThorQ::Networking::Udp::Connection::readEnd(ThorQ::Networking::IncomingMessage msg)
{
    m_connectionHandler->onMessage(msg.body);
    readHeader();
}

void ThorQ::Networking::Udp::Connection::asioClose()
{
    std::scoped_lock l(l_socket);
    if (m_socket.is_open())
    {
        m_socket.close();
    }
}
*/
