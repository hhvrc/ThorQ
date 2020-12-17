#include "crypto.h"

#include <limits>
#include <cstring>
#include <cstdint>
#include <cassert>
#include <algorithm>

void ThorQ::Crypto::RandomizeBytes(std::span<std::uint8_t> bytes)
{
    assert(sodium_init() >= 0);
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
}

void ThorQ::Crypto::reset()
{
    std::unique_lock l(m_modlock);
    reset_nolock();
}

bool ThorQ::Crypto::ready() const
{
    return m_state == State::Ready;
}

bool ThorQ::Crypto::generateKeyPair()
{
    std::unique_lock l(m_modlock);

    reset_shared_nolock();
    if (crypto_kx_keypair(m_pk.data(), m_sk.data()) != 0)
    {
        reset_nolock();
        return false;
    }

    m_state = State::GeneratedKeys;
    return true;
}

bool ThorQ::Crypto::getPublicKey(std::span<std::uint8_t, Crypto::PublicKeyLen> publicKeyOut) const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(m_modlock));
    if (m_state != State::GeneratedKeys)
    {
        return false;
    }

    std::copy(m_pk.begin(), m_pk.end(), publicKeyOut.begin());

    return true;
}

bool ThorQ::Crypto::agreeAsServer(const std::span<std::uint8_t, Crypto::PublicKeyLen> foreignKey)
{
    std::unique_lock l(m_modlock);
    if (m_state != State::GeneratedKeys)
    {
        return false;
    }

    if (crypto_kx_server_session_keys(m_rx.data(), m_tx.data(), m_pk.data(), m_sk.data(), foreignKey.data()) != 0)
    {
        reset_shared_nolock();
        return false;
    }

    m_state = State::Ready;
    return true;
}

bool ThorQ::Crypto::agreeAsClient(const std::span<std::uint8_t, Crypto::PublicKeyLen> foreignKey)
{
    std::unique_lock l(m_modlock);
    if (m_state != State::GeneratedKeys)
    {
        return false;
    }

    if (crypto_kx_client_session_keys(m_rx.data(), m_tx.data(), m_pk.data(), m_sk.data(), foreignKey.data()) != 0)
    {
        reset_shared_nolock();
        return false;
    }

    m_state = State::Ready;
    return true;
}

bool ThorQ::Crypto::encrypt(std::span<std::uint8_t> dataOut, const std::span<std::uint8_t> dataIn, std::span<std::uint8_t, Crypto::MacLen> mac, std::span<std::uint8_t, Crypto::NonceLen> nonce) const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(m_modlock));
    if (!ready() ||
        dataIn.empty() ||
        dataIn.size() != dataOut.size())
    {
        return false;
    }

    randombytes_buf(nonce.data(), nonce.size());

    if (crypto_secretbox_detached(dataOut.data(), mac.data(), dataIn.data(), dataIn.size(), nonce.data(), m_tx.data()) != 0)
    {
        return false;
    }

    return true;
}

bool ThorQ::Crypto::decrypt(std::span<std::uint8_t> dataOut, const std::span<std::uint8_t> dataIn, std::span<std::uint8_t, Crypto::MacLen> mac, std::span<std::uint8_t, Crypto::NonceLen> nonce) const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(m_modlock));
    if (!ready() ||
        dataIn.empty() ||
        dataIn.size() != dataOut.size())
    {
        return false;
    }

    if (crypto_secretbox_open_detached(dataOut.data(), dataIn.data(), mac.data(), dataIn.size(), nonce.data(), m_rx.data()) != 0)
    {
        return false;
    }

    return true;
}

void ThorQ::Crypto::reset_nolock()
{
    std::memset(m_pk.data(), 0, m_pk.size());
    std::memset(m_sk.data(), 0, m_sk.size());
    reset_shared_nolock();
    m_state = State::Uninitialized;
}

void ThorQ::Crypto::reset_shared_nolock()
{
    std::memset(m_rx.data(), 0, m_rx.size());
    std::memset(m_tx.data(), 0, m_tx.size());
}
