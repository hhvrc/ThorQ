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
struct Instance
{
    Instance() = delete;
    Instance(ENetPeer* peer)
        : m_peer(peer)
        , m_crypto(new ThorQ::Crypto())
        , l_account()
        , m_account()
        , l_systemID()
        , m_systemID()
        , m_verificationData()
        , m_cryptoState(THORQ_STATE_CRYPTO::THORQ_STATE_CRYPTO_NONE)
        , m_hwidState(THORQ_STATE_HWID::THORQ_STATE_HWID_NONE)
    {
        peer->data = this;
    }

    ENetPeer* m_peer;
    std::shared_ptr<ThorQ::Crypto> m_crypto;

    std::shared_mutex l_account;
    std::shared_ptr<ThorQ::Account> m_account;

    std::shared_mutex l_systemID;
    std::vector<std::uint8_t> m_systemID;

    std::vector<std::uint8_t> m_verificationData;

    std::atomic<THORQ_STATE_CRYPTO> m_cryptoState;
    std::atomic<THORQ_STATE_HWID>   m_hwidState;
};
}

#endif // INSTANCE_H
