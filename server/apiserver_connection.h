#ifndef APICONNECTION_H
#define APICONNECTION_H

#include "account.h"

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

namespace ThorQ {
class ApiServerConnection final : public ThorQ::Networking::TcpConnection
{
public:
    ApiServerConnection(asio::io_context& asio, asio::ip::tcp::socket socket);
    ApiServerConnection(ApiServerConnection&& other);
    ~ApiServerConnection();

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

    void onCryptoEstablished();

    void handleMessage(const void* body);

    void handleMessageAccount(const void* body);
    void handleMessageAccount_GetAccountId(const void* body);
    void handleMessageAccount_GetHashingParameters(const void* body);
    void handleMessageAccount_LoginRequest(const void* body);
    void handleMessageAccount_RegistrationRequest(const void* body);

    void handleMessageAnnouncement(const void* body);
    void handleMessageDevice(const void* body);
    void handleMessageCrypto(const void* body);
    void handleMessageFile(const void* body);
    void handleMessageFriendRequest(const void* body);
    void handleMessageGroup(const void* body);
    void handleMessageModeration(const void* body);
    void handleMessageSystemID(const void* body);
    void handleMessageUser(const void* body);
    void handleMessageVersion(const void *body);
    void handleMessageP2P(const void* body);

    bool encodeAndSend(flatbuffers::span<std::uint8_t> buffer, bool encrypt);

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
