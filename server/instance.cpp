#include "instance.h"

#include <iostream>

#include <enet.h>
#include <fmt/core.h>

#include <enums.h>
#include <crypto.h>
#include <constants.h>
#include <thorq_message.h>
#include <schemas/crypto_generated.h>
#include <schemas/session_generated.h>

#include "server.h"
#include "memorymanager.h"
#include "utils.h"
#include "account.h"

ThorQ::Instance::Instance(ENetPeer* peer)
    : m_peer(peer)
    , m_account(nullptr)
    , m_crypto(new Crypto())
    , m_verificationData()
    , m_systemID(THORQ_AUTH_SYSTEMID_LEN_MAX)
    , m_cryptoState(THORQ_STATE_CRYPTO_NONE)
    , m_hwidState(THORQ_STATE_HWID_NONE)
{
    peer->data = this;
}

ThorQ::Instance::~Instance()
{
    ENetPeer* peer = m_peer;
    m_peer = nullptr;

    std::shared_ptr<ThorQ::Account> account = m_account;
    m_account = nullptr;

    if (account != nullptr)
    {
        account->removeInstance(this);
    }

    if (peer != nullptr)
    {
        enet_peer_reset(peer);
    }
}

std::shared_ptr<ThorQ::Account> ThorQ::Instance::account() const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_account));
	return m_account;
}

void ThorQ::Instance::accountSwap(std::shared_ptr<ThorQ::Account>& account)
{
    std::unique_lock l(l_account);
	if (m_account != account)
	{
        std::swap(m_account, account);
	}
/*
    if (state != m_loginState)
    {
        m_loginState = state;

        if (state < m_loginState)
            setSessionState(THORQ_STATE_SESSION_NONE);

        if (state == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            qDebug() << tr("Sending login notification about [%1]").arg(account()->username());
            std::vector<std::uint8_t> message;
            thorq_payload_notification_pack(message, THORQ_NOTIFICATION_USER_ACTIVITY, account()->username(), activityState());
            broadcastNotification(message, true);
        }
        else if (state == THORQ_STATE_LOGIN_LOGGEDOUT)
        {
            //if (account() != nullptr)
            //	onlineInstances->remove(account()->username());

            std::vector<std::uint8_t> message;
            thorq_payload_notification_pack(message, THORQ_NOTIFICATION_USER_OFFLINE, account()->username());
            broadcastNotification(message, true);

            for (Instance* i : m_incoming_requests)
            {
                thorq_payload_ack_pack(message, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_DENIED, account()->username() + " went offline");
                i->packetSend(message, true, true);
            }

            setAccount(nullptr);
        }
    }*/
}

void ThorQ::Instance::setHwid(const std::vector<std::uint8_t>& hwid)
{
    if (m_systemID != hwid)
	{
        m_systemID = hwid;
	}
}

std::vector<std::uint8_t> ThorQ::Instance::hwid() const
{
    return m_systemID;
}

void ThorQ::Instance::setPeer(ENetPeer* peer)
{
	m_peer->data = nullptr;
	m_peer = peer;
	if (peer != nullptr)
		peer->data = this;
}

ENetPeer* ThorQ::Instance::peer() const
{
	return m_peer;
}

THORQ_STATE_CRYPTO ThorQ::Instance::cryptoState() const
{
	return m_cryptoState;
}

void ThorQ::Instance::setCryptoState(THORQ_STATE_CRYPTO state)
{
	if (state < m_cryptoState)
        setHwidState(THORQ_STATE_HWID_NONE);
    m_cryptoState = state;
}

THORQ_STATE_HWID ThorQ::Instance::hwidState() const
{
    return m_hwidState;
}

void ThorQ::Instance::setHwidState(THORQ_STATE_HWID state)
{

}

bool ThorQ::Instance::cryptoInit()
{
    if (m_crypto->generateKeyPair())
    {
        setCryptoState(THORQ_STATE_CRYPTO_ESTABLISHING);

        // TODO: Flatbuffer response: [ESTABLISH] [m_crypto->getPublicKey()]

        return true;
    }

    m_crypto->reset();
    setCryptoState(THORQ_STATE_CRYPTO_NONE);
    disconnectPeer(THORQ_DISCONNECT_REASON::CRYPTO_FAILED);

    return false;
}

bool ThorQ::Instance::cryptoEstablish(const flatbuffers::Vector<std::uint8_t>& data)
{
    if (cryptoState() == THORQ_STATE_CRYPTO_ESTABLISHING && data.size() != 0)
	{
        if (m_crypto->agree(data.data(), data.size()))
        {
            setCryptoState(THORQ_STATE_CRYPTO_VERIFYING);

            m_verificationData.resize(THORQ_CRYPTO_VERIFICATION_DATA_LEN);
            Crypto::RandomizeBytes(m_verificationData.data(), m_verificationData.size());

            // TODO: Flatbuffer response: [VERIFY] [VERIFICATION_DATA] !!! Send this message over encrypted connection !!!

			return true;
		}
	}

    m_crypto->reset();
    setCryptoState(THORQ_STATE_CRYPTO_NONE);
    disconnectPeer(THORQ_DISCONNECT_REASON::CRYPTO_FAILED);

	return false;
}


bool ThorQ::Instance::cryptoVerify(const flatbuffers::Vector<std::uint8_t>& data)
{
    if (cryptoState() == THORQ_STATE_CRYPTO_VERIFYING && data.size() == THORQ_CRYPTO_VERIFICATION_DATA_LEN && m_verificationData.size() == THORQ_CRYPTO_VERIFICATION_DATA_LEN)
	{
        if (memcmp(m_verificationData.data(), data.data(), THORQ_CRYPTO_VERIFICATION_DATA_LEN) == 0)
        {
            setCryptoState(THORQ_STATE_CRYPTO_ACTIVE);

            // TODO: Flatbuffer response: [OK] !!! Send this message over encrypted connection !!!

			return true;
		}
	}

    m_crypto->reset();
    setCryptoState(THORQ_STATE_CRYPTO_NONE);
    disconnectPeer(THORQ_DISCONNECT_REASON::CRYPTO_FAILED);

    return false;
}

ENetPacket* ThorQ::Instance::packetEncode(const uint8_t *data, std::size_t dataSize, bool encrypt, bool reliable)
{
    ENetPacket* packet = ThorQ::Memory::packetGet(dataSize, reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED);

    if (packet != nullptr)
    {
        if (encrypt)
        {
            if (!ThorQ::packetEncode(packet, data, dataSize, m_crypto))
            {
                fmt::print(stderr, "Failed to encode packet!");
                ThorQ::Memory::packetFree(packet);
                packet = nullptr;
            }
        }
        else
        {
            if (!ThorQ::packetEncode(packet, data, dataSize))
            {
                fmt::print(stderr, "Failed to encode packet!");
                ThorQ::Memory::packetFree(packet);
                packet = nullptr;
            }
        }
    }
    else
    {
        fmt::print(stderr, "Failed to allocate packet!");
    }

    return packet;
}

bool ThorQ::Instance::packetDecode(const ENetPacket *packet, std::vector<std::uint8_t>& payload)
{
    return ThorQ::packetDecode(packet, payload, m_crypto);
}

bool ThorQ::Instance::packetSend(ENetPacket *packet, THORQ_CHANNEL ch)
{
    if (packet != nullptr)
    {
        return g_server->tryQueueMessage(ThorQ::Server::QueuedMessage{m_peer, packet, ch});
    }

    return false;
}

void ThorQ::Instance::disconnectPeer(THORQ_DISCONNECT_REASON reason, bool force)
{
    if (force)
    {
        enet_peer_disconnect_now(m_peer, (std::uint32_t)reason);
    }
    else
    {
        enet_peer_disconnect(m_peer, (std::uint32_t)reason);
    }
}
