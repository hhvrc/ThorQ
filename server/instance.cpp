#include "instance.h"

#include <iostream>

#include <enet.h>
#include <enums.h>
#include <crypto.h>
#include <constants.h>
#include <thorq_message.h>
#include <thorq_payload_ack.h>
#include <thorq_payload_crypto.h>
#include <thorq_payload_session.h>

#include "singletons.h"
#include "utils.h"
#include "account.h"

ThorQ::Instance::Instance(ENetPeer* peer)
    : m_cryptoState(THORQ_STATE_CRYPTO_NONE)
    , m_authState(THORQ_STATE_AUTH_NONE)
	, m_peer(peer)
    , m_systemID()
    , m_account(nullptr)
    , m_crypto(new Crypto())
    , m_verificationData(nullptr)
{
    peer->data = this;
}

ThorQ::Instance::~Instance()
{
	if (m_peer != nullptr)
		enet_peer_reset(m_peer);
	delete m_crypto;
}

ThorQ::Account* ThorQ::Instance::account() const
{
	return m_account;
}

void ThorQ::Instance::setAccount(Account* account)
{
	if (m_account != account)
	{
		m_account = account;

        if (account->m_sessions.empty())
        {

        }
	}

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
                i->sendMessage(message, true, true);
            }

            setAccount(nullptr);
        }
    }
}

void ThorQ::Instance::setHwid(const QByteArray& hwid)
{
    if (m_systemID != hwid)
	{
        m_systemID = hwid;

	}
}

const QByteArray& ThorQ::Instance::hwid() const
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
        setAuthState(THORQ_STATE_AUTH_NONE);
    m_cryptoState = state;
}

THORQ_STATE_AUTH ThorQ::Instance::authState() const
{
    return m_authState;
}

void ThorQ::Instance::setSessionState(THORQ_STATE_SESSION state)
{
	if (state != m_sessionState)
    {
        m_sessionState = state;

        Instance* partner = m_partner;

        if (state == THORQ_STATE_SESSION_ACTIVE)
        {
            if (partner != nullptr)
            {
                partner->m_partner = this;

                partner->setSessionState(THORQ_STATE_SESSION_ACTIVE);

                // Set activity flag
                m_activityState |= THORQ_USER_ACTIVITY_FLAG_IN_SESSION;

                std::vector<std::uint8_t> message;

                thorq_payload_event_pack(message, THORQ_EVENT_SESSION_STARTED, account()->username());
                sendMessage(message, true, true);

                thorq_payload_notification_pack(message, THORQ_NOTIFICATION_USER_ACTIVITY, account()->username(), m_activityState);
                broadcastNotification(message, true);
            }
            else
            {
                setSessionState(THORQ_STATE_SESSION_NONE);
            }
        }
        else if (state == THORQ_STATE_SESSION_NONE)
        {
            if (partner != nullptr)
            {
                // clear partner
                m_partner = nullptr;

				// Clear self from partner, so that it doesnt call recursivley
                partner->m_partner = nullptr;

				// Run partner session disconnection
                partner->setSessionState(THORQ_STATE_SESSION_NONE);
            }

            // Set activity flag
            m_activityState &= ~THORQ_USER_ACTIVITY_FLAG_IN_SESSION;

            std::vector<std::uint8_t> message;

            thorq_payload_event_pack(message, THORQ_EVENT_SESSION_STOPPED, account()->username());
            sendMessage(message, true, true);

            thorq_payload_notification_pack(message, THORQ_NOTIFICATION_USER_ACTIVITY, account()->username(), m_activityState);
            broadcastNotification(message, true);
        }
    }
}

void ThorQ::Instance::cryptoInit()
{
    getCrypto()->reset();

    setCryptoState(THORQ_STATE_CRYPTO_ESTABLISHING);

    std::vector<std::uint8_t> message;
    thorq_payload_crypto_establish_pack(message, getCrypto()->publicKey());
    sendMessage(message, THORQ_CHANNEL_MAIN, true, true);
}

bool ThorQ::Instance::cryptoEstablish(const std::vector<std::uint8_t>& data)
{
    if (cryptoState() == THORQ_STATE_CRYPTO_ESTABLISHING && !data.empty())
	{
        if (getCrypto()->agree(data))
        {
            m_verificationData = new std::uint8_t[THORQ_CRYPTO_VERIFICATION_DATA_LENGTH];
			Crypto::RandomizeBytes(m_verificationData, THORQ_CRYPTO_VERIFICATION_DATA_LENGTH);

            std::vector<std::uint8_t> message;
            thorq_payload_crypto_verify_pack(message, )
            thorq_payload_crypto_pack(message, THORQ_PAYLOAD_CRYPTO_VERIFY, m_verificationData, THORQ_CRYPTO_VERIFICATION_DATA_LENGTH);
            sendMessage(message, true, true);
            setCryptoState(THORQ_STATE_CRYPTO_VERIFYING);
			return true;
		}
	}

    getCrypto()->reset();
    setCryptoState(THORQ_STATE_CRYPTO_NONE);
    disconnectPeer(THORQ_DISCONNECT_REASON_CRYPT_FAILED);

	return false;
}


bool ThorQ::Instance::cryptoVerify(const std::vector<std::uint8_t>& data)
{
    if (cryptoState() == THORQ_STATE_CRYPTO_VERIFYING && data.size() == THORQ_CRYPTO_VERIFICATION_DATA_LENGTH)
	{
		if (memcmp(&m_verificationData[0], &data[0], THORQ_CRYPTO_VERIFICATION_DATA_LENGTH) == 0)
		{
            std::vector<std::uint8_t> message;
            thorq_payload_crypto_pack(message, THORQ_PAYLOAD_CRYPTO_OK);
            sendMessage(message, true, true);
            setCryptoState(THORQ_STATE_CRYPTO_ACTIVE);

			return true;
		}
	}

    getCrypto()->reset();
    setCryptoState(THORQ_STATE_CRYPTO_NONE);
    disconnectPeer(THORQ_DISCONNECT_REASON_CRYPT_FAILED);

	return false;
}


ThorQ::Crypto* ThorQ::Instance::getCrypto()
{
	return m_crypto;
}

void ThorQ::Instance::sendMessage(std::vector<std::uint8_t>& message, bool encrypt, bool reliable)
{
	if (encrypt)
	{
        if (!thorq_payload_encode(message, m_crypto))
			return;
	}
	else
	{
        if (!thorq_payload_encode(message))
			return;
	}

    sendRaw(message, reliable);
}
void ThorQ::Instance::sendMessage(const std::vector<std::uint8_t>& message, bool encrypt, bool reliable)
{
    std::vector<std::uint8_t> copy = message;

	if (encrypt)
	{
        if (!thorq_payload_encode(copy, m_crypto))
			return;
	}
	else
	{
        if (!thorq_payload_encode(copy))
			return;
	}

    sendRaw(copy, reliable);
}

void ThorQ::Instance::sendRaw(const std::vector<std::uint8_t>& raw, THORQ_CHANNEL ch, bool reliable)
{
    enet_peer_send(m_peer, ch, enet_packet_create(raw.data(), raw.size(), reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED));
}

void ThorQ::Instance::disconnectPeer(std::uint32_t reason)
{
	enet_peer_disconnect(m_peer, reason);
}

void ThorQ::Instance::disconnectPeerForcibly(std::uint32_t reason)
{
	enet_peer_disconnect_now(m_peer, reason);
}
