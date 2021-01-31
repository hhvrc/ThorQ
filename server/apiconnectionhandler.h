#ifndef INSTANCE_H
#define INSTANCE_H

#include "account.h"

#include <networking/connectionhandler.h>
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
struct ApiConnectionHandler final : public ThorQ::Networking::ConnectionHandlerInterface
{
public:
    ApiConnectionHandler();
    ~ApiConnectionHandler();

    std::shared_ptr<ThorQ::Crypto::Encryption> crypto() const;
    void setCrypto(std::shared_ptr<ThorQ::Crypto::Encryption> crypto);

    std::shared_ptr<ThorQ::Account> account() const;
    void setAccount(std::shared_ptr<ThorQ::Account> account);

    std::shared_ptr<std::vector<std::uint8_t>> systemID() const;
    void setSystemID(std::shared_ptr<std::vector<std::uint8_t>> systemID);
private:
    void onConnect() override;
    void onDisconnect() override;
    bool onHeader(std::shared_ptr<ThorQ::Networking::MessageHeader> header) override;
    void onMessage(std::shared_ptr<std::vector<std::uint8_t>> message) override;

    std::shared_mutex l_crypto;
    std::shared_ptr<ThorQ::Crypto::Encryption> m_crypto;

    std::shared_mutex l_account;
    std::shared_ptr<ThorQ::Account> m_account;

    std::shared_mutex l_systemID;
    std::shared_ptr<std::vector<std::uint8_t>> m_systemID;

    std::array<std::uint8_t, THORQ_CRYPTO_VERIFICATION_DATA_LEN> m_verificationData;

    std::atomic<THORQ_STATE_CRYPTO> m_cryptoState;
    std::atomic<THORQ_STATE_HWID>   m_hwidState;
};
}

#endif // INSTANCE_H
