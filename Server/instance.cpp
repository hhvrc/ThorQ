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
#include <thorq_payload_notification.h>

#include "singletons.h"
#include "utils.h"
#include "instancemap.h"

ThorQ::Instance::Instance(ENetPeer* peer)
	: m_crypto(new Crypto())
	, m_activityState(0)
	, m_connectionState(THORQ_STATE_CONNECTION_DISCONNECTED)
	, m_cryptoState(THORQ_STATE_CRYPTO_NONE)
	, m_authState(THORQ_STATE_AUTH_NONE)
	, m_loginState(THORQ_STATE_LOGIN_LOGGEDOUT)
	, m_sessionState(THORQ_STATE_SESSION_NONE)
	, m_name()
	, m_hwid()
	, m_peer(peer)
	, m_partner(nullptr)
    , m_incoming_requests()
    , m_outgoing_requests()
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

std::string& ThorQ::Instance::name()
{
	return m_name;
}

const std::string& ThorQ::Instance::name() const
{
	return m_name;
}

std::vector<std::uint8_t>& ThorQ::Instance::hwid()
{
	return m_hwid;
}

const std::vector<uint8_t>& ThorQ::Instance::hwid() const
{
	return m_hwid;
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
    std::vector<std::uint8_t> response;

    if (m_partner != nullptr)
    {
        thorq_message_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_DENIED, "You are already in another session");
        sendMessage(response, true, true);
		return;
	}

    if (target == this)
    {
        thorq_message_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_DENIED, "Cannot request on self");
        sendMessage(response, true, true);
		return;
	}

    if (target->isInSession())
    {
        thorq_message_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_DENIED, target->name() + " is already in another session");
        sendMessage(response, true, true);
		return;
	}

    // Spam prevention
    if (!m_outgoing_requests.insert(target).second)
    {
        thorq_message_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_NO_CHANGE);
        sendMessage(response, true, true);
        return;
    }

    target->m_incoming_requests.insert(this);

    thorq_message_event_pack(response, THORQ_EVENT_SESSION_REQUESTED, name());
    target->sendMessage(response, true, true);

    thorq_message_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_IN_PROGRESS, "Request sent");
    this->sendMessage(response, true, true);
}
bool ThorQ::Instance::requestAcceptFrom(Instance* sender)
{
    std::vector<std::uint8_t> response;

	if (sender == this)
	{
        thorq_message_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_ACCEPT, THORQ_COMMAND_ACK_RESULT_DENIED, "Cannot start session with self");
        sendMessage(response, true, true);
		return false;
	}

    if (m_incoming_requests.extract(sender).empty())
	{
        thorq_message_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_ACCEPT, THORQ_COMMAND_ACK_RESULT_DENIED, " Request from " + sender->name() + " is invalid / never got sent");
        sendMessage(response, true, true);
		return false;
	}
    sender->m_outgoing_requests.extract(this);

    if (m_partner != nullptr)
    {
        thorq_message_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_ACCEPT, THORQ_COMMAND_ACK_RESULT_DENIED, "You are already in another session");
        sendMessage(response, true, true);
        return false;
    }

    if (!sender->isInSession())
	{
        thorq_message_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_ACCEPT, THORQ_COMMAND_ACK_RESULT_DENIED, sender->name() + " is already in another session");
        sendMessage(response, true, true);
		return false;
    }

	m_partner = sender;

	setSessionState(THORQ_STATE_SESSION_ACTIVE);

    thorq_message_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_OK, "Request accepted");
    m_partner->sendMessage(response, true, true);
    thorq_message_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_ACCEPT, THORQ_COMMAND_ACK_RESULT_OK, "Session started");
    this->sendMessage(response, true, true);

	return true;
}
bool ThorQ::Instance::requestDenyFrom(ThorQ::Instance *sender)
{
    std::vector<std::uint8_t> response;

	if (sender == this)
	{
        thorq_message_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_DENY, THORQ_COMMAND_ACK_RESULT_DENIED, "Cannot deny session with self");
        sendMessage(response, true, true);
		return false;
	}

    if (m_incoming_requests.extract(sender).empty())
	{
        thorq_message_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_DENY, THORQ_COMMAND_ACK_RESULT_DENIED, " Request from " + sender->name() + " is invalid / never got sent");
        sendMessage(response, true, true);
		return false;
	}
    sender->m_outgoing_requests.extract(this);

    thorq_message_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_DENIED, "Request denied");
    sender->sendMessage(response, true, true);
    thorq_message_command_ack_pack(response, THORQ_COMMAND_ID_SESSION_DENY, THORQ_COMMAND_ACK_RESULT_OK);
    this->sendMessage(response, true, true);

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

        std::vector<std::uint8_t> message;
        thorq_message_notification_pack(message, THORQ_NOTIFICATION_USER_ACTIVITY, name(), m_activityState);
        broadcastNotification(message, true);
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

        std::vector<std::uint8_t> message;
        thorq_message_notification_pack(message, THORQ_NOTIFICATION_USER_ACTIVITY, name(), m_activityState);
        broadcastNotification(message, true);
    }
}

void ThorQ::Instance::setActivityState(uint8_t state)
{
    m_activityState = state;

    std::vector<std::uint8_t> message;
    thorq_message_notification_pack(message, THORQ_NOTIFICATION_USER_ACTIVITY, name(), m_activityState);
    broadcastNotification(message, true);
}

uint8_t ThorQ::Instance::activityState() const
{
    return m_activityState;
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

THORQ_STATE_CONNECTION ThorQ::Instance::connectionState() const
{
	return m_connectionState;
}

void ThorQ::Instance::setConnectionState(THORQ_STATE_CONNECTION state)
{
    printf("SetConnectionState %i\n", state - THORQ_STATE_CONNECTION_DISCONNECTED);
	if (state < m_connectionState)
        setCryptoState(THORQ_STATE_CRYPTO_NONE);
	m_connectionState = state;
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

void ThorQ::Instance::setAuthState(THORQ_STATE_AUTH state)
{
    if (state < m_authState)
        setLoginState(THORQ_STATE_LOGIN_LOGGEDOUT);
    m_authState = state;
}
THORQ_STATE_LOGIN ThorQ::Instance::loginState() const
{
	return m_loginState;
}
void ThorQ::Instance::setLoginState(THORQ_STATE_LOGIN state)
{
	if (state != m_loginState)
    {
		m_loginState = state;

		if (state < m_loginState)
			setSessionState(THORQ_STATE_SESSION_NONE);

		if (state == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            printf("Sending login notification about [%s]\n", name().c_str());
            std::vector<std::uint8_t> message;
            thorq_message_notification_pack(message, THORQ_NOTIFICATION_USER_ACTIVITY, name(), activityState());
            broadcastNotification(message, true);
		}
		else if (state == THORQ_STATE_LOGIN_LOGGEDOUT)
        {
            if (!name().empty())
                registeredInstances->remove(name());

            std::vector<std::uint8_t> message;
            thorq_message_notification_pack(message, THORQ_NOTIFICATION_USER_OFFLINE, name());
            broadcastNotification(message, true);

			for (Instance* i : m_incoming_requests)
			{
                thorq_message_command_ack_pack(message, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_DENIED, name() + " went offline");
                i->sendMessage(message, true, true);
			}

            name().clear();
		}
	}
}
THORQ_STATE_SESSION ThorQ::Instance::sessionState() const
{
	return m_sessionState;
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

                thorq_message_event_pack(message, THORQ_EVENT_SESSION_STARTED, name());
                sendMessage(message, true, true);

                thorq_message_notification_pack(message, THORQ_NOTIFICATION_USER_ACTIVITY, name(), m_activityState);
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

            thorq_message_event_pack(message, THORQ_EVENT_SESSION_STOPPED, name());
            sendMessage(message, true, true);

            thorq_message_notification_pack(message, THORQ_NOTIFICATION_USER_ACTIVITY, name(), m_activityState);
            broadcastNotification(message, true);
        }
    }
}

void ThorQ::Instance::cryptoInit()
{
    getCrypto()->reset();

    setCryptoState(THORQ_STATE_CRYPTO_ESTABLISHING);

    std::vector<std::uint8_t> message;
    thorq_message_crypto_pack(message, THORQ_CRYPTO_ESTABLISH, getCrypto()->publicKey());
    sendMessage(message, false, true);
}

bool ThorQ::Instance::cryptoEstablish(const std::vector<std::uint8_t>& data)
{
    if (cryptoState() == THORQ_STATE_CRYPTO_ESTABLISHING && !data.empty())
	{
        if (getCrypto()->agree(data))
        {
			Crypto::RandomizeBytes(m_verificationData, THORQ_CRYPTO_VERIFICATION_DATA_LENGTH);
            std::vector<std::uint8_t> message;
            thorq_message_crypto_pack(message, THORQ_CRYPTO_VERIFY, m_verificationData, THORQ_CRYPTO_VERIFICATION_DATA_LENGTH);
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


bool ThorQ::Instance::cryptoVerify(const std::vector<std::uint8_t>& data)
{
    if (cryptoState() == THORQ_STATE_CRYPTO_VERIFYING && data.size() == THORQ_CRYPTO_VERIFICATION_DATA_LENGTH)
	{
		if (memcmp(&m_verificationData[0], &data[0], THORQ_CRYPTO_VERIFICATION_DATA_LENGTH) == 0)
		{
            std::vector<std::uint8_t> message;
            thorq_message_crypto_pack(message, THORQ_CRYPTO_OK);
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


ThorQ::Crypto* ThorQ::Instance::getCrypto()
{
	return m_crypto;
}

void ThorQ::Instance::sendMessage(std::vector<uint8_t>& message, bool encrypt, bool reliable)
{
	if (encrypt)
	{
        if (!thorq_message_encode(message, m_crypto))
			return;
	}
	else
	{
        if (!thorq_message_encode(message))
			return;
	}

    sendRaw(message, reliable);
}
void ThorQ::Instance::sendMessage(const std::vector<uint8_t>& message, bool encrypt, bool reliable)
{
    std::vector<std::uint8_t> copy = message;

	if (encrypt)
	{
		if (!thorq_message_encode(copy, m_crypto))
			return;
	}
	else
	{
		if (!thorq_message_encode(copy))
			return;
	}

    sendRaw(copy, reliable);
}

void ThorQ::Instance::sendRaw(const std::vector<uint8_t>& raw, bool reliable)
{
    enet_peer_send(m_peer, reliable ? 0 : 1, enet_packet_create(raw.data(), raw.size(), reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED));
}

void ThorQ::Instance::disconnect(uint32_t reason)
{
	enet_peer_disconnect(m_peer, reason);
}

void ThorQ::Instance::disconnectForcibly(uint32_t reason)
{
	enet_peer_disconnect_now(m_peer, reason);
}
