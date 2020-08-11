#include "instance.h"

#include <iostream>

#include <enet.h>
#include <enums.h>
#include <crypto.h>
#include <constants.h>

#include <thorq_message.h>
#include <thorq_payload_event.h>
#include <thorq_payload_crypto.h>
#include <thorq_payload_command_ack.h>

#include "singletons.h"
#include "utils.h"
#include "instancemap.h"

ThorQ::Instance::Instance(ENetPeer* peer)
	: m_crypto(new Crypto())
	, m_connectionState(THORQ_CONNECTION_STATE_DISCONNECTED)
	, m_cryptoState(THORQ_CRYPTO_STATE_NONE)
	, m_loginState(THORQ_LOGIN_STATE_LOGGEDOUT)
	, m_sessionState(THORQ_SESSION_STATE_NONE)
    , m_name("")
	, m_peer(peer)
	, m_partner(nullptr)
	, m_requestedPartner(nullptr)
	, m_verificationData()
{
    peer->data = this;
}

ThorQ::Instance::~Instance()
{
	if (m_peer != nullptr)
		enet_peer_reset(m_peer);
	delete m_crypto;
}

void ThorQ::Instance::setHwid(const std::string& hwid)
{
    m_hwid = hwid;
}

const std::string& ThorQ::Instance::hwid() const
{
    return m_hwid;
}

bool ThorQ::Instance::hasHwid()
{
    return !m_hwid.empty();
}

void ThorQ::Instance::setName(const std::string& newName)
{
	m_name = newName;
}

const std::string& ThorQ::Instance::name() const
{
	return m_name;
}

bool ThorQ::Instance::hasName()
{
	return !m_name.empty();
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

void ThorQ::Instance::requestOn(Instance* target)
{
    thorq_payload_t response;

	// Spam prevention
	if (m_requestedPartner == target)
    {
        thorq_payload_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_NO_CHANGE);
        sendPayload(&response, true, true);
		return;
    }

    if (m_partner != nullptr)
    {
        thorq_payload_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_DENIED, "You are already in another session");
        sendPayload(&response, true, true);
		return;
	}

    if (target == this)
    {
        thorq_payload_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_DENIED, "Cannot request on self");
        sendPayload(&response, true, true);
		return;
	}

    if (target->isInSession())
    {
        thorq_payload_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_DENIED, target->name() + " is already in another session");
        sendPayload(&response, true, true);
		return;
	}

	m_requestedPartner = target;

    thorq_payload_event_pack(response, THORQ_EVENT_SESSION_REQUESTED, name());
    sendPayload(&response, true, true);

    thorq_payload_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_IN_PROGRESS, "Request sent");
    sendPayload(&response, true, true);
}
bool ThorQ::Instance::requestAcceptFrom(Instance* sender)
{
    thorq_payload_t response;

	if (sender == this)
	{
        thorq_payload_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_ACCEPT, THORQ_COMMAND_ACK_RESULT_DENIED, "Cannot start session with self");
        sendPayload(&response, true, true);
		return false;
	}

	if (sender->m_requestedPartner != this)
	{
        thorq_payload_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_ACCEPT, THORQ_COMMAND_ACK_RESULT_DENIED, " Request from " + sender->name() + " is invalid / never got sent");
        sendPayload(&response, true, true);
		return false;
	}

    if (!sender->isInSession())
	{
        thorq_payload_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_ACCEPT, THORQ_COMMAND_ACK_RESULT_DENIED, sender->name() + " is already in another session");
        sendPayload(&response, true, true);
		return false;
	}

	if (m_partner != nullptr)
	{
        thorq_payload_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_ACCEPT, THORQ_COMMAND_ACK_RESULT_DENIED, "You are already in another session");
        sendPayload(&response, true, true);
		return false;
	}

	m_partner = sender;

	setSessionState(THORQ_SESSION_STATE_ACTIVE);

    thorq_payload_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_OK, "Request accepted");
    m_partner->sendPayload(&response, true, true);
    thorq_payload_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_ACCEPT, THORQ_COMMAND_ACK_RESULT_OK, "Session started");
    this->sendPayload(&response, true, true);

	return true;
}

bool ThorQ::Instance::requestDenyFrom(ThorQ::Instance *sender)
{
    thorq_payload_t response;

	if (sender == this)
	{
        thorq_payload_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_DENY, THORQ_COMMAND_ACK_RESULT_DENIED, "Cannot deny session with self");
        sendPayload(&response, true, true);
		return false;
	}

	if (sender->m_requestedPartner != this)
	{
        thorq_payload_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_DENY, THORQ_COMMAND_ACK_RESULT_DENIED, " Request from " + sender->name() + " is invalid / never got sent");
        sendPayload(&response, true, true);
		return false;
	}

    thorq_payload_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_DENIED, "Request denied");
    sender->sendPayload(&response, true, true);
    thorq_payload_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_DENY, THORQ_COMMAND_ACK_RESULT_OK, "Session denied");
    this->sendPayload(&response, true, true);

	return true;
}
ThorQ::Instance* ThorQ::Instance::partner() const
{
	return m_partner;
}

void ThorQ::Instance::setIsInSteamVR(bool value)
{
    if (isInSteamVR() != value)
    {
        if (value)
            m_activityState |= THORQ_USER_ACTIVITY_FLAG_COLLAR_PRESENT;
        else
            m_activityState &= ~THORQ_USER_ACTIVITY_FLAG_COLLAR_PRESENT;
    }
}

void ThorQ::Instance::setHasCollar(bool value)
{
    if (hasCollar() != value)
    {
        if (value)
            m_activityState |= THORQ_USER_ACTIVITY_FLAG_COLLAR_PRESENT;
        else
            m_activityState &= ~THORQ_USER_ACTIVITY_FLAG_COLLAR_PRESENT;
    }
}

bool ThorQ::Instance::isInSession() const
{
    return (m_activityState & THORQ_USER_ACTIVITY_FLAG_IN_SESSION) != 0;
}

bool ThorQ::Instance::isInSteamVR() const
{
    return (m_activityState & THORQ_USER_ACTIVITY_FLAG_OPENVR_RUNNING) != 0;
}

bool ThorQ::Instance::hasCollar() const
{
    return (m_activityState & THORQ_USER_ACTIVITY_FLAG_COLLAR_PRESENT) != 0;
}

thorq_connection_state_t ThorQ::Instance::connectionState() const
{
	return m_connectionState;
}

void ThorQ::Instance::setConnectionState(thorq_connection_state_t state)
{
	if (state < m_connectionState)
        setCryptoState(THORQ_CRYPTO_STATE_NONE);
	m_connectionState = state;
}

thorq_crypto_state_t ThorQ::Instance::cryptoState() const
{
	return m_cryptoState;
}

void ThorQ::Instance::setCryptoState(thorq_crypto_state_t state)
{
	if (state < m_cryptoState)
        setAuthState(THORQ_AUTH_STATE_NONE);
    m_cryptoState = state;
}

thorq_auth_state_t ThorQ::Instance::authState() const
{
    return m_authState;
}

void ThorQ::Instance::setAuthState(thorq_auth_state_t state)
{
    if (state < m_authState)
        setLoginState(THORQ_LOGIN_STATE_LOGGEDOUT);
    m_authState = state;
}
thorq_login_state_t ThorQ::Instance::loginState() const
{
	return m_loginState;
}
void ThorQ::Instance::setLoginState(thorq_login_state_t state)
{
	if (state != m_loginState)
	{
		m_loginState = state;

		if (state == THORQ_LOGIN_STATE_LOGGEDIN)
        {
			thorq_payload_t payload;
			// TODO: notify about user_online
            broadcastAnnouncement(&payload, true);
		}
		else if (state == THORQ_LOGIN_STATE_LOGGEDOUT)
        {
			thorq_payload_t payload;
			// TODO: notify about user_offline
            broadcastAnnouncement(&payload, true);
		}

		if (state < m_loginState)
			setSessionState(THORQ_SESSION_STATE_NONE);
	}
}
thorq_session_state_t ThorQ::Instance::sessionState() const
{
	return m_sessionState;
}
void ThorQ::Instance::setSessionState(thorq_session_state_t state)
{
	if (state != m_sessionState)
	{
		m_sessionState = state;

		if (state == THORQ_SESSION_STATE_NONE)
		{
			Instance* partner = m_partner;

			if (partner != nullptr)
			{
				// Clear self from partner, so that it doesnt call recursivley
				m_partner->m_partner = nullptr;

				// clear partner
				m_partner = nullptr;

				// Run partner session disconnection
				m_partner->setSessionState(THORQ_SESSION_STATE_NONE);
			}

			// If we have already are notifying users that someone went offline then there is no use in telling them that they left a session, that is obvious
            if (m_loginState == THORQ_LOGIN_STATE_LOGGEDIN)
			{
				thorq_payload_t payload;

				thorq_payload_event_pack(payload, THORQ_EVENT_SESSION_STOPPED, name());
				sendPayload(&payload, true, true);

				thorq_payload_event_pack(payload, THORQ_EVENT_SESSION_STOPPED, name());
                broadcastAnnouncement(&payload, true);
            }
		}
		else if (state == THORQ_SESSION_STATE_ACTIVE)
		{
			if (m_partner != nullptr)
			{
				m_partner->m_partner = this;

				m_partner->setSessionState(THORQ_SESSION_STATE_ACTIVE);

				thorq_payload_t payload;

				thorq_payload_event_pack(payload, THORQ_EVENT_SESSION_STARTED, name());
				sendPayload(&payload, true, true);

				thorq_payload_event_pack(payload, THORQ_EVENT_SESSION_STARTED, name());
                broadcastAnnouncement(&payload, true);
			}
			else
			{
				setSessionState(THORQ_SESSION_STATE_NONE);
			}
		}
	}
}

void ThorQ::Instance::cryptoInit()
{
    getCrypto()->reset();

    setCryptoState(THORQ_CRYPTO_STATE_ESTABLISHING);

	thorq_payload_t payload;
    thorq_payload_crypto_pack(payload, THORQ_CRYPTO_ESTABLISH, getCrypto()->publicKey());
    sendPayload(&payload, false, true);
}

bool ThorQ::Instance::cryptoEstablish(const std::vector<std::uint8_t>& data)
{
    if (cryptoState() == THORQ_CRYPTO_STATE_ESTABLISHING && !data.empty())
	{
        if (getCrypto()->agree(data))
        {
			Crypto::RandomizeBytes(m_verificationData, THORQ_CRYPTO_VERIFICATION_DATA_LENGTH);
			thorq_payload_t payload;
			thorq_payload_crypto_pack(payload, THORQ_CRYPTO_VERIFY, m_verificationData, THORQ_CRYPTO_VERIFICATION_DATA_LENGTH);
            sendPayload(&payload, true, true);
            setCryptoState(THORQ_CRYPTO_STATE_VERIFYING);
			return true;
		}
	}

    getCrypto()->reset();
    setCryptoState(THORQ_CRYPTO_STATE_NONE);
    disconnect(THORQ_DISCONNECT_REASON_CRYPT_FAILED);

	return false;
}


bool ThorQ::Instance::cryptoVerify(const std::vector<std::uint8_t>& data)
{
    if (cryptoState() == THORQ_CRYPTO_STATE_VERIFYING && data.size() == THORQ_CRYPTO_VERIFICATION_DATA_LENGTH)
	{
		if (memcmp(&m_verificationData[0], &data[0], THORQ_CRYPTO_VERIFICATION_DATA_LENGTH) == 0)
		{
			thorq_payload_t payload;
			thorq_payload_crypto_pack(payload, THORQ_CRYPTO_OK);
			sendPayload(&payload, true, true);
            setCryptoState(THORQ_CRYPTO_STATE_ACTIVE);

			return true;
		}
	}

    getCrypto()->reset();
    setCryptoState(THORQ_CRYPTO_STATE_NONE);
    disconnect(THORQ_DISCONNECT_REASON_CRYPT_FAILED);

	return false;
}


ThorQ::Crypto* ThorQ::Instance::getCrypto()
{
	return m_crypto;
}

void ThorQ::Instance::sendPayload(const thorq_payload_t* payload, bool encrypt, bool reliable)
{
	std::vector<std::uint8_t> message;

	thorq_payload_pack(*payload, message);

    sendMessage(message, encrypt, reliable);
}

void ThorQ::Instance::sendMessage(const std::vector<uint8_t>& message, bool encrypt, bool reliable)
{
	std::vector<std::uint8_t> data;

	if (encrypt)
        thorq_message_encode(message, data, getCrypto());
	else
		thorq_message_encode(message, data);

	enet_peer_send(m_peer, reliable ? 0 : 1, enet_packet_create(data.data(), data.size(), reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED));
}

void ThorQ::Instance::disconnect(uint32_t reason)
{
	enet_peer_disconnect(m_peer, reason);
}

void ThorQ::Instance::disconnectForcibly(uint32_t reason)
{
	enet_peer_disconnect_now(m_peer, reason);
}
