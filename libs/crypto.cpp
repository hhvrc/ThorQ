#include "crypto.h"

#include <limits>
#include <cstring>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <execution>

void ThorQ::Crypto::RandomizeBytes(std::span<std::uint8_t> bytes)
{
    randombytes_buf(bytes.data(), bytes.size());
}

ThorQ::Crypto::Crypto()
    : m_state(State::Uninitialized)
    , m_modlock()
    , m_pk{0}
    , m_sk{0}
    , m_rx{0}
    , m_tx{0}
{
    assert(sodium_init() >= 0);
}

ThorQ::Crypto::~Crypto()
{
    reset();
}

void ThorQ::Crypto::reset()
{
    std::unique_lock l(m_modlock);
    reset_nolock();
}

bool ThorQ::Crypto::ready() const
{
    return m_state != State::Uninitialized;
}

bool ThorQ::Crypto::generateKeyPair()
{
    std::unique_lock l(m_modlock);
    return crypto_kx_keypair(m_pk.data(), m_sk.data()) == 0;
}

bool ThorQ::Crypto::getPublicKey(std::span<std::uint8_t> publicKeyOut) const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(m_modlock));
    if (ready() && publicKeyOut.size() == Crypto::PublicKeyLen)
    {
        std::copy(m_pk.begin(), m_pk.end(), publicKeyOut.begin());
        return true;
    }

    return false;
}

bool ThorQ::Crypto::agree(const std::span<std::uint8_t> foreignKey)
{
    std::unique_lock l(m_modlock);
    if (ready() &&
        foreignKey.size() == Crypto::PublicKeyLen)
    {
        return crypto_kx_server_session_keys(m_rx.data(), m_tx.data(), m_pk.data(), m_sk.data(), foreignKey.data()) == 0;
    }

    return false;
}

bool ThorQ::Crypto::encrypt(std::span<std::uint8_t> dataOut, const std::span<std::uint8_t> dataIn, std::span<std::uint8_t> mac, std::span<std::uint8_t> nonce) const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(m_modlock));
    if (ready() &&
        !dataIn.empty() &&
        dataIn.size() == dataOut.size() &&
        mac.size() == Crypto::MacLen &&
        nonce.size() == Crypto::NonceLen)
    {
        randombytes_buf(nonce.data(), nonce.size());
        return crypto_secretbox_detached(dataOut.data(), mac.data(), dataIn.data(), dataIn.size(), nonce.data(), m_tx.data()) == 0;
    }

    return false;
}

bool ThorQ::Crypto::decrypt(std::span<std::uint8_t> dataOut, const std::span<std::uint8_t> dataIn, std::span<std::uint8_t> mac, std::span<std::uint8_t> nonce) const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(m_modlock));
    if (ready() &&
        !dataIn.empty() &&
        dataIn.size() == dataOut.size() &&
        mac.size() == Crypto::MacLen &&
        nonce.size() == Crypto::NonceLen)
    {
        return crypto_secretbox_open_detached(dataOut.data(), mac.data(), dataIn.data(), dataIn.size(), nonce.data(), m_rx.data()) == 0;
    }

    return false;
}

void ThorQ::Crypto::reset_nolock()
{
    m_state = State::Uninitialized;
    std::memset(m_pk.data(), 0, m_pk.size());
    std::memset(m_sk.data(), 0, m_sk.size());
    std::memset(m_rx.data(), 0, m_rx.size());
    std::memset(m_tx.data(), 0, m_tx.size());
}
