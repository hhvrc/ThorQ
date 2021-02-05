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

    void handleMessageAccount(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageAnnouncement(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageDevice(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageCrypto(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageFile(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageFriendRequest(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageGroup(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageModeration(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageSystemID(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageUser(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageVersion(const void *body, flatbuffers::Verifier fbsVerifier);
    void handleMessageP2P(const void* body, flatbuffers::Verifier fbsVerifier);

    void encodeAndSend(flatbuffers::span<std::uint8_t> buffer, bool encrypt);

    std::vector<std::uint8_t> m_buffer;

    ThorQ::Crypto::Encryption m_crypto;

    std::shared_mutex l_account;
    std::shared_ptr<ThorQ::Account> m_account;

    std::shared_mutex l_systemID;
    std::shared_ptr<std::vector<std::uint8_t>> m_systemID;

    std::atomic<THORQ_STATE_CRYPTO> m_cryptoState;
    std::atomic<THORQ_STATE_HWID>   m_hwidState;
};
}

#endif // APICONNECTION_H
