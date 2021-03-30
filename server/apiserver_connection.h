#ifndef APICONNECTION_H
#define APICONNECTION_H

#include "account.h"

#include <networking/tcpconnection.h>
#include <cryptography/encryption.h>
#include <messagehandlingcontext.h>
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

    void handleMessage(HandlerContext& context);
    void createErrorMessage(HandlerContext& context, const char* error, std::uint64_t requestId);
    bool sendContextData(HandlerContext& context);

    void handleMessageAccount(HandlerContext& context);
    void handleMessageAccount_GetAccountId(HandlerContext& context);
    void handleMessageAccount_GetHashingSalt(HandlerContext& context);
    void handleMessageAccount_GetHashingParameters(HandlerContext& context);
    void handleMessageAccount_LoginRequest(HandlerContext& context);
    void handleMessageAccount_RegistrationRequest(HandlerContext& context);
    void handleMessageAccount_Recover(HandlerContext& context);
    void handleMessageAccount_Delete(HandlerContext& context);
    void handleMessageAccount_Logout(HandlerContext& context);
    void handleMessageAccount_SetUserName(HandlerContext& context);
    void handleMessageAccount_SetPassword(HandlerContext& context);
    void handleMessageAccount_SetEmail(HandlerContext& context);
    void handleMessageAccount_SetImage(HandlerContext& context);

    void handleMessageAnnouncement(HandlerContext& context);
    void handleMessageDevice(HandlerContext& context);
    void handleMessageCrypto(HandlerContext& context);
    void handleMessageFile(HandlerContext& context);
    void handleMessageFriendRequest(HandlerContext& context);
    void handleMessageGroup(HandlerContext& context);
    void handleMessageModeration(HandlerContext& context);
    void handleMessageSystemID(HandlerContext& context);
    void handleMessageUser(HandlerContext& context);
    void handleMessageVersion(HandlerContext& context);
    void handleMessageP2P(HandlerContext& context);

    bool encodeAndSend(flatbuffers::span<std::uint8_t> buffer, bool encrypt);

    std::vector<std::uint8_t> m_buffer;

    bool crypto_ok = false;
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
