#ifndef UDPCONNECTION_H
#define UDPCONNECTION_H

#include "abstractconnection.h"
#include "typedefs_global.h"

#include "asio_common.h"

#include <memory>
#include <mutex>
#include <cstdint>

namespace ThorQ {
namespace Networking {
namespace Udp {
/*
class Connection final : public ThorQ::Networking::Connection
{
public:
    Connection(asio::io_context& asio, asio::ip::udp::socket socket);
    ~Connection();

    void connect(const asio::ip::udp::resolver::results_type& endpoints);
    void disconnect() override;

    bool isOpen() const;

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
    asio::ip::udp::socket m_socket;
};
*/
}
}
}

#endif // UDPCONNECTION_H
