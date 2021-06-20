#ifndef APICONNECTION_H
#define APICONNECTION_H

#include "account.h"
#include "apiserver_connection.h"
#include "messagehandlingcontext.h"
#include "api_endpoints/endpoint_account.h"
#include "api_endpoints/endpoint_announcement.h"
#include "api_endpoints/endpoint_crypto.h"
#include "api_endpoints/endpoint_device.h"
#include "api_endpoints/endpoint_error.h"
#include "api_endpoints/endpoint_file.h"
#include "api_endpoints/endpoint_friendrequest.h"
#include "api_endpoints/endpoint_group.h"
#include "api_endpoints/endpoint_message.h"
#include "api_endpoints/endpoint_moderation.h"
#include "api_endpoints/endpoint_p2p.h"
#include "api_endpoints/endpoint_systemid.h"
#include "api_endpoints/endpoint_user.h"
#include "api_endpoints/endpoint_version.h"

#include <networking/tcpconnection.h>
#include <cryptography/encryption.h>
#include <typedefs_global.h>
#include <constants.h>
#include <enums.h>

#include <array>
#include <vector>
#include <memory>
#include <shared_mutex>
#include <atomic>
#include <cstdint>

void setServer(std::shared_ptr<ThorQ::ApiServer> server);

namespace ThorQ {
class ApiServerConnection final : public ThorQ::Networking::TcpConnection
{
public:
    ApiServerConnection(asio::io_context& asio, asio::ip::tcp::socket socket);
    ApiServerConnection(ApiServerConnection&& other);
    ~ApiServerConnection();

    std::vector<std::uint8_t>& buffer() { return m_buffer; }

    ThorQ::Crypto::Encryption& crypto() { return m_crypto; }
    const ThorQ::Crypto::Encryption& crypto() const { return m_crypto; }

    bool encodeAndSend(flatbuffers::span<std::uint8_t> buffer, bool encrypt);

    std::shared_ptr<ThorQ::Account> account() const;
    void setAccount(std::shared_ptr<ThorQ::Account> account);

    std::shared_ptr<std::vector<std::uint8_t>> systemID() const;
    void setSystemID(std::shared_ptr<std::vector<std::uint8_t>> systemID);
private:
    // Event handlers
    void onError(std::error_code ec) override;
    void onConnect(std::vector<std::uint8_t> address, std::uint16_t port) override;
    void onDisconnect() override;
    bool onHeader(const ThorQ::Encoding::MessageHeader* header) override;
    void onMessage(std::shared_ptr<std::vector<std::uint8_t>> message) override;

    void handleMessage(HandlerContext& context);

    std::vector<std::uint8_t> m_buffer;
    ThorQ::Crypto::Encryption m_crypto;

    std::atomic_bool m_hasAccountID;
    std::shared_mutex l_account;
    std::shared_ptr<ThorQ::Account> m_account;

    std::atomic_bool m_hasSystemID;
    std::shared_mutex l_systemID;
    std::shared_ptr<std::vector<std::uint8_t>> m_systemID;
};
}

#endif // APICONNECTION_H
