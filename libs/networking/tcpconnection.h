#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "abstractconnection.h"
#include "typedefs_global.h"

#include "asio_common.h"

#include <memory>
#include <mutex>
#include <cstdint>

namespace ThorQ {
namespace Networking {
namespace Tcp {
class Connection final : public ThorQ::Networking::Connection
{
public:
    Connection(asio::io_context& asio, asio::ip::tcp::socket socket);
    ~Connection();

    void accept();
    void connect(const asio::ip::tcp::resolver::results_type& endpoints, std::function<void()> onConnect);
    void disconnect() override;

    bool isConnected() const;

    void send(std::shared_ptr<std::vector<std::uint8_t>> data) override;
private:
    void writeHeader(ThorQ::Networking::Message msg) override;
    void writeBody(ThorQ::Networking::Message msg) override;
    void writeEnd() override;

    void readHeader() override;
    void readBody(ThorQ::Networking::IncomingMessage msg) override;
    void readEnd(ThorQ::Networking::IncomingMessage msg) override;

    void asioClose() override;

    std::mutex l_socket;
    asio::ip::tcp::socket m_socket;
};
}
}
}
#endif // TCPCONNECTION_H
