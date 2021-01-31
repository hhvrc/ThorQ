#ifndef APICONNECTION_H
#define APICONNECTION_H

#include <networking/connectionhandler.h>
#include <cryptography/encryption.h>
#include <typedefs_global.h>
#include <constants.h>

#include <QObject>

#include <array>
#include <vector>
#include <memory>
#include <shared_mutex>
#include <cstdint>

namespace ThorQ {
class ApiConnectionHandler : public ThorQ::Networking::ConnectionHandlerInterface, public QObject
{
public:
    ApiConnectionHandler();
    ~ApiConnectionHandler();
private:
    // Event handlers
    void onConnect() override;
    void onDisconnect() override;
    bool onHeader(std::shared_ptr<ThorQ::Networking::MessageHeader> header) override;
    void onMessage(std::shared_ptr<std::vector<std::uint8_t>> message) override;

    void requestCrypto();
    void handleMessageVersion(const void *body, flatbuffers::Verifier fbsVerifier);
    void handleMessageCrypto(const void* body, flatbuffers::Verifier fbsVerifier);

    void encodeAndSend(flatbuffers::span<std::uint8_t> buffer, bool encrypt);

    ThorQ::Crypto::Encryption m_crypto;

    std::mutex l_buffer;
    std::vector<std::uint8_t> m_buffer;
};
}

#endif // APICONNECTION_H
