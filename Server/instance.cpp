#include "instance.h"

#include <iostream>

#include <enet.h>
#include <enums.h>
#include <crypto.h>

ThorQ::Instance::Instance(ENetPeer* peer)
    : m_crypto(new Crypto())
    , m_clientState(ThorQ::ClientState::Disconnected)
    , m_cryptoState(ThorQ::CryptoState::None)
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
        SendEncrypted(ThorQ::MessageEnums::ACKNOWLEDGE_Denied, "You are already in another session");
		return;
	}

	if (target == this) {
        SendEncrypted(ThorQ::MessageEnums::ACKNOWLEDGE_Denied, "Cannot request on self");
		return;
	}

	if (target->HasPartner())
	{
        SendEncrypted(ThorQ::MessageEnums::ACKNOWLEDGE_Denied, target->Name() + " is already in another session");
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
        SendEncrypted(ThorQ::MessageEnums::ACKNOWLEDGE_Denied, "Cannot start session with self");
        return false;
	}

    if (sender->m_requestedPartner != this)
	{
        SendEncrypted(ThorQ::MessageEnums::ACKNOWLEDGE_Denied, " Request from " + sender->Name() + " is invalid / never got sent");
        return false;
	}

	if (!sender->HasPartner())
	{
        SendEncrypted(ThorQ::MessageEnums::ACKNOWLEDGE_Denied, sender->Name() + " is already in another session");
        return false;
	}

	if (m_partner != nullptr)
	{
        SendEncrypted(ThorQ::MessageEnums::ACKNOWLEDGE_Denied, "You are already in another session");
        return false;
	}

	this->m_partner = sender;
    sender->m_partner = this;

    sender->SendEncrypted(ThorQ::MessageEnums::NOTIFY_SessionAccepted, this->Name());
    this->SendEncrypted(ThorQ::MessageEnums::NOTIFY_SessionAccepted, sender->Name());

    return true;
}

bool ThorQ::Instance::RequestDenyFrom(ThorQ::Instance *sender)
{
    if (sender == this)
    {
        SendEncrypted(ThorQ::MessageEnums::ACKNOWLEDGE_Denied, "Cannot deny session with self");
        return false;
    }

    if (sender->m_requestedPartner != this)
    {
        SendEncrypted(ThorQ::MessageEnums::ACKNOWLEDGE_Denied, " Request from " + sender->Name() + " is invalid / never got sent");
        return false;
    }

    sender->SendEncrypted(ThorQ::MessageEnums::NOTIFY_SessionDenied, this->Name());
    this->SendEncrypted(ThorQ::MessageEnums::NOTIFY_SessionDenied, sender->Name());

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

    m_partner->SendEncrypted(ThorQ::MessageEnums::NOTIFY_SessionEnded, m_name);
    SendEncrypted(ThorQ::MessageEnums::NOTIFY_SessionEnded, m_partner->Name());

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

int ThorQ::Instance::ClientState() const
{
    return m_clientState;
}

void ThorQ::Instance::SetClientState(int state)
{
    m_clientState = state;
}

int ThorQ::Instance::CryptoState() const
{
    return m_cryptoState;
}

void ThorQ::Instance::SetCryptoState(int state)
{
    m_cryptoState = state;
}

void ThorQ::Instance::SendHeartbeat()
{
    std::uint8_t flag = GetFlag(true);
    enet_peer_send(m_peer, 1, enet_packet_create(&flag, sizeof(std::uint8_t), ENET_PACKET_FLAG_UNSEQUENCED));
}

void ThorQ::Instance::SendRaw(std::uint32_t meta)
{
    SendRaw(reinterpret_cast<std::uint8_t*>(&meta), sizeof(std::uint32_t));
}
void ThorQ::Instance::SendRaw(std::uint32_t meta, const std::string &message, bool unreliable)
{
    std::size_t len = sizeof(std::uint32_t) + message.length();
    std::uint8_t* data = new std::uint8_t[len];
    memcpy(data, &meta, sizeof(std::uint32_t));
    memcpy(data + sizeof(std::uint32_t), message.data(), message.length());
    SendRaw(data, len, unreliable);
}
void ThorQ::Instance::SendRaw(const std::vector<std::uint8_t>& data, bool unreliable)
{
    std::size_t size = data.size() + 1;
    std::uint8_t* dat = new std::uint8_t[size];
    dat[0] = GetFlag();
    memcpy(dat + 1, data.data(), data.size());

    enet_peer_send(m_peer, unreliable ? 1 : 0, enet_packet_create(dat, size, unreliable ? ENET_PACKET_FLAG_UNSEQUENCED : ENET_PACKET_FLAG_RELIABLE));
}
void ThorQ::Instance::SendRaw(const std::uint8_t* data, std::size_t len, bool unreliable) { SendRaw(std::vector<std::uint8_t>(data, data + len), unreliable); }

void ThorQ::Instance::SendEncrypted(uint32_t meta)
{
    SendEncrypted(reinterpret_cast<std::uint8_t*>(&meta), sizeof(std::uint32_t));
}
void ThorQ::Instance::SendEncrypted(std::uint32_t meta, const std::string &message, bool unreliable)
{
    std::size_t len = sizeof(std::uint32_t) + message.length();
    std::uint8_t* data = new std::uint8_t[len];
    memcpy(data, &meta, sizeof(std::uint32_t));
    memcpy(data + sizeof(std::uint32_t), message.data(), message.length());
    SendEncrypted(data, len, unreliable);
}
void ThorQ::Instance::SendEncrypted(const std::vector<std::uint8_t>& data, bool unreliable) { SendEncrypted(data.data(), data.size(), unreliable); }
void ThorQ::Instance::SendEncrypted(const std::uint8_t* data, std::size_t len, bool unreliable)
{
    std::vector<std::uint8_t> encrypted = m_crypto->Encrypt(data, len);

    encrypted.insert(encrypted.begin(), GetFlag());

    enet_peer_send(m_peer, unreliable ? 1 : 0, enet_packet_create(encrypted.data(), encrypted.size(), unreliable ? ENET_PACKET_FLAG_UNSEQUENCED : ENET_PACKET_FLAG_RELIABLE));
}

void ThorQ::Instance::CryptoInit()
{
    SetClientState(ThorQ::ClientState::Connecting);
    SetCryptoState(ThorQ::CryptoState::Establishing);
    SendRaw(GetCrypto()->PublicKey());
}

void ThorQ::Instance::CryptoEstablish(const std::uint8_t* data, std::size_t size)
{
	if (GetCrypto()->Agree(data, size))
    {
        if (CryptoState() != ThorQ::CryptoState::Establishing)
            return;
        SetCryptoState(ThorQ::CryptoState::Verifying);
		Crypto::RandomizeBytes(m_verificationData, 256);
		SendEncrypted(m_verificationData, 256);
        return;
    }

    GetCrypto()->Reset();
    SetCryptoState(ThorQ::CryptoState::None);
    SendRaw(ThorQ::MessageEnums::ACKNOWLEDGE_Error);
}


bool ThorQ::Instance::CryptoVerify(const std::uint8_t* data, std::size_t size)
{
	if (CryptoState() != ThorQ::CryptoState::Verifying || size != 256)
        return false;

	if (memcmp(m_verificationData, data, 256) == 0)
	{
        SetCryptoState(ThorQ::CryptoState::Ok);
        SendEncrypted(ThorQ::MessageEnums::ACKNOWLEDGE_OK);

        if (ClientState() == ThorQ::ClientState::Connecting)
            SetClientState(ThorQ::ClientState::Connected);

        return true;
    }

    GetCrypto()->Reset();
    SetCryptoState(ThorQ::CryptoState::None);
    SendRaw(ThorQ::MessageEnums::ACKNOWLEDGE_Error);

    return false;
}


ThorQ::Crypto* ThorQ::Instance::GetCrypto()
{
    return m_crypto;
}

std::uint8_t ThorQ::Instance::GetFlag(bool withHeartbeat)
{
    std::uint8_t flag = withHeartbeat ? ThorQ::PreEncryptionFlag::HEARTBEAT : 0;

    switch (CryptoState()) {
    case ThorQ::CryptoState::Establishing:
        flag |= ThorQ::PreEncryptionFlag::CRYPT_ESTABLISH;
        break;
    case ThorQ::CryptoState::Verifying:
        flag |= ThorQ::PreEncryptionFlag::CRYPT_VERIFY;
        break;
    case ThorQ::CryptoState::Ok:
        flag |= ThorQ::PreEncryptionFlag::CRYPT_OK;
        break;
	}

    return flag;
}
