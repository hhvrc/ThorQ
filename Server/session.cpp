#include "session.h"

#include <iostream>

#include <QDebug>

#include <enet.h>
#include <enums.h>
#include <crypto.h>
#include <constants.h>
#include <thorq_message.h>
#include <thorq_payload_event.h>
#include <thorq_payload_crypto.h>
#include <thorq_payload_notification.h>

#include "singletons.h"
#include "utils.h"
#include "account.h"

ThorQ::Session::Session(ENetPeer* peer, QObject* parent)
	: QObject(parent)
	, m_crypto(new Crypto())
	, m_activityState(0)
	, m_connectionState(THORQ_STATE_CONNECTION_DISCONNECTED)
	, m_cryptoState(THORQ_STATE_CRYPTO_NONE)
	, m_authState(THORQ_STATE_AUTH_NONE)
	, m_loginState(THORQ_STATE_LOGIN_LOGGEDOUT)
	, m_sessionState(THORQ_STATE_SESSION_NONE)
	, m_peer(peer)
	, m_hwid()
	, m_account(nullptr)
	, m_partner(nullptr)
    , m_incoming_requests()
    , m_outgoing_requests()
	, m_verificationData()
{
    peer->data = this;
}

ThorQ::Session::~Session()
{
	if (m_peer != nullptr)
		enet_peer_reset(m_peer);
	delete m_crypto;
}

ThorQ::Account* ThorQ::Session::account() const
{
	return m_account;
}

void ThorQ::Session::setAccount(Account* account)
{
	if (m_account != account)
	{
		m_account = account;

	}
}

void ThorQ::Session::setHwid(const QByteArray& hwid)
{
	if (m_hwid != hwid)
	{
		m_hwid = hwid;

	}
}

const QByteArray& ThorQ::Session::hwid() const
{
	return m_hwid;
}

void ThorQ::Session::setPeer(ENetPeer* peer)
{
	m_peer->data = nullptr;
	m_peer = peer;
	if (peer != nullptr)
		peer->data = this;
}

ENetPeer* ThorQ::Session::peer() const
{
	return m_peer;
}

void ThorQ::Session::requestOn(Session* target)
{
    std::vector<std::uint8_t> response;

    if (m_partner != nullptr)
    {
        thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_DENIED, "You are already in another session");
        sendMessage(response, true, true);
		return;
	}

    if (target == this)
    {
        thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_DENIED, "Cannot request on self");
        sendMessage(response, true, true);
		return;
	}

    if (target->isInSession())
    {
        thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_DENIED, target->account()->username() + " is already in another session");
        sendMessage(response, true, true);
		return;
	}

    // Spam prevention
	if (m_outgoing_requests.contains(target))
    {
        thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_NO_CHANGE);
        sendMessage(response, true, true);
        return;
    }
	m_outgoing_requests.insert(target);

    target->m_incoming_requests.insert(this);

    thorq_payload_event_pack(response, THORQ_EVENT_SESSION_REQUESTED, account()->username());
    target->sendMessage(response, true, true);

    thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_IN_PROGRESS, "Request sent");
    this->sendMessage(response, true, true);
}
bool ThorQ::Session::requestAcceptFrom(Session* sender)
{
    std::vector<std::uint8_t> response;

	if (sender == this)
	{
        thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_ACCEPT, THORQ_COMMAND_ACK_RESULT_DENIED, "Cannot start session with self");
        sendMessage(response, true, true);
		return false;
	}

	if (!m_incoming_requests.remove(sender))
	{
        thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_ACCEPT, THORQ_COMMAND_ACK_RESULT_DENIED, " Request from " + sender->account()->username() + " is invalid / never got sent");
        sendMessage(response, true, true);
		return false;
	}
	sender->m_outgoing_requests.remove(this);

    if (m_partner != nullptr)
    {
        thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_ACCEPT, THORQ_COMMAND_ACK_RESULT_DENIED, "You are already in another session");
        sendMessage(response, true, true);
        return false;
    }

    if (!sender->isInSession())
	{
        thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_ACCEPT, THORQ_COMMAND_ACK_RESULT_DENIED, sender->account()->username() + " is already in another session");
        sendMessage(response, true, true);
		return false;
    }

	m_partner = sender;

	setSessionState(THORQ_STATE_SESSION_ACTIVE);

    thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_OK, "Request accepted");
    m_partner->sendMessage(response, true, true);
    thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_ACCEPT, THORQ_COMMAND_ACK_RESULT_OK, "Session started");
    this->sendMessage(response, true, true);

	return true;
}
bool ThorQ::Session::requestDenyFrom(ThorQ::Session *sender)
{
    std::vector<std::uint8_t> response;

	if (sender == this)
	{
        thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_DENY, THORQ_COMMAND_ACK_RESULT_DENIED, "Cannot deny session with self");
        sendMessage(response, true, true);
		return false;
	}

	if (!m_incoming_requests.remove(sender))
	{
        thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_DENY, THORQ_COMMAND_ACK_RESULT_DENIED, " Request from " + sender->account()->username() + " is invalid / never got sent");
        sendMessage(response, true, true);
		return false;
	}
	sender->m_outgoing_requests.remove(this);

    thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_DENIED, "Request denied");
    sender->sendMessage(response, true, true);
    thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_DENY, THORQ_COMMAND_ACK_RESULT_OK);
    this->sendMessage(response, true, true);

	return true;
}
ThorQ::Session* ThorQ::Session::partner() const
{
	return m_partner;
}

void ThorQ::Session::setIsInSteamVR(bool value)
{
    if (isInSteamVR() != value)
    {
        if (value)
            m_activityState |= THORQ_USER_ACTIVITY_FLAG_COLLAR_PRESENT;
        else
            m_activityState &= ~THORQ_USER_ACTIVITY_FLAG_COLLAR_PRESENT;

        std::vector<std::uint8_t> message;
        thorq_payload_notification_pack(message, THORQ_NOTIFICATION_USER_ACTIVITY, account()->username(), m_activityState);
        broadcastNotification(message, true);
    }
}

void ThorQ::Session::setHasCollar(bool value)
{
    if (hasCollar() != value)
    {
        if (value)
            m_activityState |= THORQ_USER_ACTIVITY_FLAG_COLLAR_PRESENT;
        else
            m_activityState &= ~THORQ_USER_ACTIVITY_FLAG_COLLAR_PRESENT;

        std::vector<std::uint8_t> message;
        thorq_payload_notification_pack(message, THORQ_NOTIFICATION_USER_ACTIVITY, account()->username(), m_activityState);
        broadcastNotification(message, true);
    }
}

void ThorQ::Session::setActivityState(uint8_t state)
{
    m_activityState = state;

    std::vector<std::uint8_t> message;
    thorq_payload_notification_pack(message, THORQ_NOTIFICATION_USER_ACTIVITY, account()->username(), m_activityState);
    broadcastNotification(message, true);
}

uint8_t ThorQ::Session::activityState() const
{
    return m_activityState;
}

bool ThorQ::Session::isInSession() const
{
    return (m_activityState & THORQ_USER_ACTIVITY_FLAG_IN_SESSION) != 0;
}

bool ThorQ::Session::isInSteamVR() const
{
    return (m_activityState & THORQ_USER_ACTIVITY_FLAG_OPENVR_RUNNING) != 0;
}

bool ThorQ::Session::hasCollar() const
{
    return (m_activityState & THORQ_USER_ACTIVITY_FLAG_COLLAR_PRESENT) != 0;
}

THORQ_STATE_CONNECTION ThorQ::Session::connectionState() const
{
	return m_connectionState;
}

void ThorQ::Session::setConnectionState(THORQ_STATE_CONNECTION state)
{
    qDebug() << "SetConnectionState" << (state - THORQ_STATE_CONNECTION_DISCONNECTED);
	if (state < m_connectionState)
        setCryptoState(THORQ_STATE_CRYPTO_NONE);
	m_connectionState = state;
}

THORQ_STATE_CRYPTO ThorQ::Session::cryptoState() const
{
	return m_cryptoState;
}

void ThorQ::Session::setCryptoState(THORQ_STATE_CRYPTO state)
{
	if (state < m_cryptoState)
        setAuthState(THORQ_STATE_AUTH_NONE);
    m_cryptoState = state;
}

THORQ_STATE_AUTH ThorQ::Session::authState() const
{
    return m_authState;
}

void ThorQ::Session::setAuthState(THORQ_STATE_AUTH state)
{
    if (state < m_authState)
        setLoginState(THORQ_STATE_LOGIN_LOGGEDOUT);
    m_authState = state;
}
THORQ_STATE_LOGIN ThorQ::Session::loginState() const
{
	return m_loginState;
}
void ThorQ::Session::setLoginState(THORQ_STATE_LOGIN state)
{
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

            for (Session* i : m_incoming_requests)
			{
                thorq_payload_ack_pack(message, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_DENIED, account()->username() + " went offline");
                i->sendMessage(message, true, true);
			}

			setAccount(nullptr);
		}
	}
}
THORQ_STATE_SESSION ThorQ::Session::sessionState() const
{
	return m_sessionState;
}
void ThorQ::Session::setSessionState(THORQ_STATE_SESSION state)
{
	if (state != m_sessionState)
    {
        m_sessionState = state;

        Session* partner = m_partner;

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

void ThorQ::Session::cryptoInit()
{
    getCrypto()->reset();

    setCryptoState(THORQ_STATE_CRYPTO_ESTABLISHING);

    std::vector<std::uint8_t> message;
    thorq_payload_crypto_pack(message, THORQ_PAYLOAD_CRYPTO_ESTABLISH, getCrypto()->publicKey());
    sendMessage(message, false, true);
}

bool ThorQ::Session::cryptoEstablish(const std::vector<std::uint8_t>& data)
{
    if (cryptoState() == THORQ_STATE_CRYPTO_ESTABLISHING && !data.empty())
	{
        if (getCrypto()->agree(data))
        {
			Crypto::RandomizeBytes(m_verificationData, THORQ_CRYPTO_VERIFICATION_DATA_LENGTH);
            std::vector<std::uint8_t> message;
            thorq_payload_crypto_pack(message, THORQ_PAYLOAD_CRYPTO_VERIFY, m_verificationData, THORQ_CRYPTO_VERIFICATION_DATA_LENGTH);
            sendMessage(message, true, true);
            setCryptoState(THORQ_STATE_CRYPTO_VERIFYING);
			return true;
		}
	}

    getCrypto()->reset();
    setCryptoState(THORQ_STATE_CRYPTO_NONE);
    disconnect(THORQ_DISCONNECT_REASON_CRYPT_FAILED);

	return false;
}


bool ThorQ::Session::cryptoVerify(const std::vector<std::uint8_t>& data)
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
    disconnect(THORQ_DISCONNECT_REASON_CRYPT_FAILED);

	return false;
}


ThorQ::Crypto* ThorQ::Session::getCrypto()
{
	return m_crypto;
}

void ThorQ::Session::sendMessage(std::vector<uint8_t>& message, bool encrypt, bool reliable)
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
void ThorQ::Session::sendMessage(const std::vector<uint8_t>& message, bool encrypt, bool reliable)
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

void ThorQ::Session::sendRaw(const std::vector<uint8_t>& raw, bool reliable)
{
    enet_peer_send(m_peer, reliable ? 0 : 1, enet_packet_create(raw.data(), raw.size(), reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED));
}

void ThorQ::Session::disconnect(uint32_t reason)
{
	enet_peer_disconnect(m_peer, reason);
}

void ThorQ::Session::disconnectForcibly(uint32_t reason)
{
	enet_peer_disconnect_now(m_peer, reason);
}
