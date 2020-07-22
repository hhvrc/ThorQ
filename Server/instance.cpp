#include "instance.h"

#include <iostream>

#include <enet.h>
#include <enums.h>
#include <crypto.h>
#include <constants.h>

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
	enet_peer_ping_interval(peer, ENET_PEER_PING_INTERVAL);
	enet_peer_get_rtt(peer);
	peer->
}

ThorQ::Instance::~Instance()
{
	delete m_crypto;
}

void ThorQ::Instance::SetName(const std::string& newName)
{
	m_name = newName;
}

const std::string& ThorQ::Instance::Name() const
{
	return m_name;
}

bool ThorQ::Instance::HasName()
{
	return !m_name.empty();
}

void ThorQ::Instance::SetPeer(ENetPeer* peer)
{
	m_peer->data = nullptr;
	m_peer = peer;
	peer->data = this;
}

ENetPeer* ThorQ::Instance::Peer() const
{
	return m_peer;
}

void ThorQ::Instance::RequestOn(Instance* target)
{
	// Spam prevention
	if (m_requestedPartner == target)
		return;

	if (m_partner != nullptr) {
		SendEncrypted(ThorQ::MessageContentEnums::ACKNOWLEDGE_Denied, "You are already in another session");
		return;
	}

	if (target == this) {
		SendEncrypted(ThorQ::MessageContentEnums::ACKNOWLEDGE_Denied, "Cannot request on self");
		return;
	}

	if (target->HasPartner())
	{
		SendEncrypted(ThorQ::MessageContentEnums::ACKNOWLEDGE_Denied, target->Name() + " is already in another session");
		return;
	}

	m_requestedPartner = target;
	target->SendEncrypted(SESSION_Request, m_name);
	SendEncrypted(ACKNOWLEDGE_OK, "Request sent");
}
bool ThorQ::Instance::RequestAcceptFrom(Instance* sender)
{
	if (sender == this)
	{
		SendEncrypted(ThorQ::MessageContentEnums::ACKNOWLEDGE_Denied, "Cannot start session with self");
		return false;
	}

	if (sender->m_requestedPartner != this)
	{
		SendEncrypted(ThorQ::MessageContentEnums::ACKNOWLEDGE_Denied, " Request from " + sender->Name() + " is invalid / never got sent");
		return false;
	}

	if (!sender->HasPartner())
	{
		SendEncrypted(ThorQ::MessageContentEnums::ACKNOWLEDGE_Denied, sender->Name() + " is already in another session");
		return false;
	}

	if (m_partner != nullptr)
	{
		SendEncrypted(ThorQ::MessageContentEnums::ACKNOWLEDGE_Denied, "You are already in another session");
		return false;
	}

	this->m_partner = sender;
	sender->m_partner = this;

	sender->SendEncrypted(ThorQ::MessageContentEnums::NOTIFY_SessionAccepted, this->Name());
	this->SendEncrypted(ThorQ::MessageContentEnums::NOTIFY_SessionAccepted, sender->Name());

	return true;
}

bool ThorQ::Instance::RequestDenyFrom(ThorQ::Instance *sender)
{
	if (sender == this)
	{
		SendEncrypted(ThorQ::MessageContentEnums::ACKNOWLEDGE_Denied, "Cannot deny session with self");
		return false;
	}

	if (sender->m_requestedPartner != this)
	{
		SendEncrypted(ThorQ::MessageContentEnums::ACKNOWLEDGE_Denied, " Request from " + sender->Name() + " is invalid / never got sent");
		return false;
	}

	sender->SendEncrypted(ThorQ::MessageContentEnums::NOTIFY_SessionDenied, this->Name());
	this->SendEncrypted(ThorQ::MessageContentEnums::NOTIFY_SessionDenied, sender->Name());

	return true;
}
ThorQ::Instance* ThorQ::Instance::Partner() const
{
	return m_partner;
}
void ThorQ::Instance::ClearPartner()
{
	if (m_partner == nullptr)
		return;

	m_partner->SendEncrypted(ThorQ::MessageContentEnums::NOTIFY_SessionEnded, m_name);
	SendEncrypted(ThorQ::MessageContentEnums::NOTIFY_SessionEnded, m_partner->Name());

	m_partner->m_partner = nullptr;
	m_partner = nullptr;
}

bool ThorQ::Instance::HasPartner()
{
	return m_partner != nullptr;
}

void ThorQ::Instance::SetHasCollar(bool hasCollar)
{
	m_hasCollar = hasCollar;
}

bool ThorQ::Instance::HasCollar() const
{
	return m_hasCollar;
}

thorq_connection_state_t ThorQ::Instance::ConnectionState() const
{
	return m_connectionState;
}
void ThorQ::Instance::SetConnectionState(thorq_connection_state_t state)
{
	if (state < m_connectionState)
		SetCryptoState(THORQ_CRYPTO_STATE_NONE);
	m_connectionState = state;
}
thorq_crypto_state_t ThorQ::Instance::CryptoState() const
{
	return m_cryptoState;
}
void ThorQ::Instance::SetCryptoState(thorq_crypto_state_t state)
{
	if (state < m_cryptoState)
		SetLoginState(THORQ_LOGIN_STATE_LOGGEDOUT);
	m_cryptoState = state;
}
thorq_login_state_t ThorQ::Instance::LoginState() const
{
	return m_loginState;
}
void ThorQ::Instance::SetLoginState(thorq_login_state_t state)
{
	if (state < m_loginState)
		SetSessionState(THORQ_SESSION_STATE_NONE);
	m_loginState = state;
}
thorq_session_state_t ThorQ::Instance::SessionState() const
{
	return m_sessionState;
}
void ThorQ::Instance::SetSessionState(thorq_session_state_t state)
{
	m_sessionState = state;
}

void ThorQ::Instance::CryptoInit()
{
    GetCrypto()->reset();
	SetCryptoState(THORQ_CRYPTO_STATE_ESTABLISHING);
    SendRaw(GetCrypto()->publicKey());
}

bool ThorQ::Instance::CryptoEstablish(const std::vector<std::uint8_t>& data)
{
	if (CryptoState() == THORQ_CRYPTO_STATE_ESTABLISHING && !data.empty())
	{
        if (GetCrypto()->agree(data))
		{
			SetCryptoState(THORQ_CRYPTO_STATE_VERIFYING);
			Crypto::RandomizeBytes(m_verificationData, MESSAGE_PAYLOAD_SIZE);
			SendEncrypted(m_verificationData, MESSAGE_PAYLOAD_SIZE);
			return true;
		}
	}

    GetCrypto()->reset();
	SetCryptoState(THORQ_CRYPTO_STATE_NONE);
	SendRaw(ThorQ::MessageContentEnums::ACKNOWLEDGE_Error);

	return false;
}


bool ThorQ::Instance::CryptoVerify(const std::vector<std::uint8_t>& data)
{
	if (CryptoState() == THORQ_CRYPTO_STATE_VERIFYING && size == MESSAGE_PAYLOAD_MAX)
	{
		if (memcmp(m_verificationData, data, MESSAGE_PAYLOAD_MAX) == 0)
		{
			SetCryptoState(THORQ_CRYPTO_STATE_ACTIVE);
			SendEncrypted(ThorQ::MessageContentEnums::ACKNOWLEDGE_OK);

			return true;
		}
	}

    GetCrypto()->reset();
	SetCryptoState(THORQ_CRYPTO_STATE_NONE);
	SendRaw(ThorQ::MessageContentEnums::ACKNOWLEDGE_Error);

	return false;
}


ThorQ::Crypto* ThorQ::Instance::GetCrypto()
{
	return m_crypto;
}
