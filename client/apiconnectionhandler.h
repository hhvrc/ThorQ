#ifndef APICONNECTION_H
#define APICONNECTION_H

#include <networking/connectionhandler.h>
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
    void onConnect() override;
    void onDisconnect() override;
    bool onHeader(std::shared_ptr<ThorQ::Networking::MessageHeader> header) override;
    void onMessage(std::shared_ptr<std::vector<std::uint8_t>> message) override;

    std::shared_mutex l_crypto;
    std::shared_ptr<ThorQ::Crypto> m_crypto;

    std::array<std::uint8_t, THORQ_CRYPTO_VERIFICATION_DATA_LEN> m_verificationData;
};
}

#endif // APICONNECTION_H
