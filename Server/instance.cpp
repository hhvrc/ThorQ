#include "instance.h"

#include <iostream>

#include <enet.h>
#include <enums.h>
#include <crypto.h>
#include <constants.h>

#include <thorq_message.h>
#include <thorq_payload_crypto.h>
#include <thorq_payload_command_ack.h>

ThorQ::Instance::Instance(ENetPeer* peer)
	: m_crypto(new Crypto())
	, m_connectionState(THORQ_CONNECTION_STATE_DISCONNECTED)
	, m_cryptoState(THORQ_CRYPTO_STATE_NONE)
	, m_loginState(THORQ_LOGIN_STATE_LOGGEDOUT)
	, m_sessionState(THORQ_SESSION_STATE_NONE)
	, m_name("")
	, m_hasCollar(false)
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
	peer->data = this;
}

ENetPeer* ThorQ::Instance::peer() const
{
	return m_peer;
}

void ThorQ::Instance::requestOn(Instance* target)
{
	// Spam prevention
	if (m_requestedPartner == target)
		return;

	if (m_partner != nullptr) {
		thorq_payload_t payload;
        thorq_payload_command_ack_pack();
        SendEncrypted(ACKNOWLEDGE_Denied, "You are already in another session");
		return;
	}

	if (target == this) {
		SendEncrypted(ThorQ::MessageContentEnums::ACKNOWLEDGE_Denied, "Cannot request on self");
		return;
	}

    if (target->hasPartner())
	{
        SendEncrypted(ThorQ::MessageContentEnums::ACKNOWLEDGE_Denied, target->name() + " is already in another session");
		return;
	}

	m_requestedPartner = target;
	target->SendEncrypted(SESSION_Request, m_name);
	SendEncrypted(ACKNOWLEDGE_OK, "Request sent");
}
bool ThorQ::Instance::requestAcceptFrom(Instance* sender)
{
	if (sender == this)
	{
		SendEncrypted(ThorQ::MessageContentEnums::ACKNOWLEDGE_Denied, "Cannot start session with self");
		return false;
	}

	if (sender->m_requestedPartner != this)
	{
        SendEncrypted(ThorQ::MessageContentEnums::ACKNOWLEDGE_Denied, " Request from " + sender->name() + " is invalid / never got sent");
		return false;
	}

    if (!sender->hasPartner())
	{
        SendEncrypted(ThorQ::MessageContentEnums::ACKNOWLEDGE_Denied, sender->name() + " is already in another session");
		return false;
	}

	if (m_partner != nullptr)
	{
		SendEncrypted(ThorQ::MessageContentEnums::ACKNOWLEDGE_Denied, "You are already in another session");
		return false;
	}

	this->m_partner = sender;
	sender->m_partner = this;

    sender->SendEncrypted(ThorQ::MessageContentEnums::NOTIFY_SessionAccepted, this->name());
    this->SendEncrypted(ThorQ::MessageContentEnums::NOTIFY_SessionAccepted, sender->name());

	return true;
}

bool ThorQ::Instance::requestDenyFrom(ThorQ::Instance *sender)
{
	if (sender == this)
	{
		SendEncrypted(ThorQ::MessageContentEnums::ACKNOWLEDGE_Denied, "Cannot deny session with self");
		return false;
	}

	if (sender->m_requestedPartner != this)
	{
        SendEncrypted(ThorQ::MessageContentEnums::ACKNOWLEDGE_Denied, " Request from " + sender->name() + " is invalid / never got sent");
		return false;
	}

    sender->SendEncrypted(ThorQ::MessageContentEnums::NOTIFY_SessionDenied, this->name());
    this->SendEncrypted(ThorQ::MessageContentEnums::NOTIFY_SessionDenied, sender->name());

	return true;
}
ThorQ::Instance* ThorQ::Instance::partner() const
{
	return m_partner;
}
void ThorQ::Instance::clearPartner()
{
	if (m_partner == nullptr)
		return;

	thorq_payload_t payload;
	m_partner->SendEncrypted(ThorQ::MessageContentEnums::NOTIFY_SessionEnded, m_name);
    SendEncrypted(ThorQ::MessageContentEnums::NOTIFY_SessionEnded, m_partner->name());

	m_partner->m_partner = nullptr;
	m_partner = nullptr;
}

bool ThorQ::Instance::hasPartner()
{
	return m_partner != nullptr;
}

void ThorQ::Instance::setHasCollar(bool hasCollar)
{
	m_hasCollar = hasCollar;
}

bool ThorQ::Instance::hasCollar() const
{
	return m_hasCollar;
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
	if (state < m_loginState)
        setSessionState(THORQ_SESSION_STATE_NONE);
	m_loginState = state;
}
thorq_session_state_t ThorQ::Instance::sessionState() const
{
	return m_sessionState;
}
void ThorQ::Instance::setSessionState(thorq_session_state_t state)
{
	m_sessionState = state;
}

void ThorQ::Instance::cryptoInit()
{
    getCrypto()->reset();

	thorq_payload_t payload;
    thorq_payload_crypto_pack(payload, THORQ_CRYPTO_ESTABLISH, getCrypto()->publicKey());
    sendPayload(payload, true, true);
    setCryptoState(THORQ_CRYPTO_STATE_ESTABLISHING);
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
            sendPayload(payload, true, true);
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
            sendPayload(payload, true, true);
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

void ThorQ::Instance::sendPayload(const thorq_payload_t& payload, bool encrypt, bool reliable)
{
	std::vector<std::uint8_t> message;

	thorq_payload_pack(payload, message);

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
