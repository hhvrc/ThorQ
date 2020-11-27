#include "instance.h"

#include "account.h"

ThorQ::Instance::Instance(ENetPeer *peer)
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

ThorQ::Instance::~Instance()
{
    m_peer->data = nullptr;

    ThorQ::Account* account = m_account.get();
    if (account != nullptr)
    {
        account->removeInstance(this);
    }
}
