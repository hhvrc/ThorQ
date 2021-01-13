#ifndef INSTANCE_H
#define INSTANCE_H

#include <cstdint>
#include <vector>
#include <memory>
#include <atomic>
#include <shared_mutex>

#include <enet.h>
#include <flatbuffers/flatbuffers.h>

#include <enums.h>
#include <crypto.h>
#include <constants.h>
#include <typedefs_global.h>

#include "typedefs_server.h"

namespace ThorQ {
class Instance
{
    Instance() = delete;
public:
    Instance(ENetPeer* peer);
    ~Instance();

    ENetPeer* m_peer;
    ThorQ::Crypto m_crypto;

    std::shared_mutex l_account;
    std::shared_ptr<ThorQ::Account> m_account;

    std::shared_mutex l_systemID;
    std::vector<std::uint8_t> m_systemID;

    std::array<std::uint8_t, THORQ_CRYPTO_VERIFICATION_DATA_LEN> m_verificationData;

    std::atomic<THORQ_STATE_CRYPTO> m_cryptoState;
    std::atomic<THORQ_STATE_HWID>   m_hwidState;
};
}

#endif // INSTANCE_H
