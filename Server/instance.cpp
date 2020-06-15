#include "instance.h"

#include <iostream>

#include <botan/hex.h>
#include <botan/ecdh.h>
#include <botan/chacha.h>
#include <botan/pubkey.h>
#include <botan/base64.h>
#include <botan/bcrypt.h>
#include <botan/system_rng.h>
#include <botan/stream_cipher.h>

#include "enet.h"
#include "enums.h"
#include "crypto.h"

using namespace ThorQ;

Instance::Instance(ENetPeer* peer) :
	m_name(""),
	m_peer(peer),
	m_partner(nullptr),
	m_requestTarget(nullptr),
	m_hasCollar(false)
{
	peer->data = this;
	m_crypto = new Crypto();
}

Instance::~Instance()
{
	delete m_crypto;
}

void Instance::SetName(const std::string& newName)
{
	m_name = newName;
}

const std::string& Instance::Name() const
{
	return m_name;
}

bool Instance::HasName()
{
	return !m_name.empty();
}

void Instance::SetPeer(ENetPeer* peer)
{
	m_peer->data = nullptr;
	m_peer = peer;
	peer->data = this;
}

ENetPeer* Instance::Peer() const
{
	return m_peer;
}

void Instance::RequestOn(Instance* target)
{
	// Spam prevention
	if (m_requestTarget == target)
		return;

	if (m_partner != nullptr) {
		SendEncMessage(ACKNOWLEDGE_Denied, "You are already in another session");
		return;
	}

	if (target == this) {
		SendEncMessage(ACKNOWLEDGE_Denied, "Cannot request on self");
		return;
	}

	if (target->HasPartner())
	{
		SendEncMessage(ACKNOWLEDGE_Denied, target->Name() + " is already in another session");
		return;
	}

	m_requestTarget = target;
	target->SendEncMessage(SESSION_Request, m_name);
	SendEncMessage(ACKNOWLEDGE_OK, "Request sent");
}
void Instance::RequestAcceptFrom(Instance* sender)
{
	if (sender == this)
	{
		SendEncMessage(ACKNOWLEDGE_Denied, "Cannot start session with self");
		return;
	}

	if (sender->m_requestTarget != this)
	{
		SendEncMessage(ACKNOWLEDGE_Denied, " Request from " + sender->Name() + " is invalid / never got sent");
		return;
	}

	if (!sender->HasPartner())
	{
		SendEncMessage(ACKNOWLEDGE_Denied, sender->Name() + " is already in another session");
		return;
	}

	if (m_partner != nullptr)
	{
		SendEncMessage(ACKNOWLEDGE_Denied, "You are already in another session");
		return;
	}

	this->m_partner = sender;
	sender->m_partner = this;

	sender->SendEncMessage(NOTIFY_SessionStarted, this->Name());
	this->SendEncMessage(NOTIFY_SessionStarted, sender->Name());
}
Instance* Instance::Partner() const
{
	return m_partner;
}
void Instance::ClearPartner()
{
	if (m_partner == nullptr)
		return;

	m_partner->SendEncMessage(NOTIFY_SessionEnded, m_name);
	SendEncMessage(NOTIFY_SessionEnded, m_partner->Name());

	m_partner->m_partner = nullptr;
	m_partner = nullptr;
}

bool Instance::HasPartner()
{
	return m_partner != nullptr;
}

void Instance::SetHasCollar(bool hasCollar)
{
	m_hasCollar = hasCollar;
}

bool Instance::HasCollar() const
{
	return m_hasCollar;
}

void Instance::SendRaw(const std::vector<uint8_t>& data, bool unreliable)
{
	enet_peer_send(m_peer, unreliable ? 1 : 0, enet_packet_create(data.data(), data.size(), unreliable ? ENET_PACKET_FLAG_UNSEQUENCED : ENET_PACKET_FLAG_RELIABLE));
}

void Instance::SendRaw(const uint8_t* data, std::size_t len, bool unreliable)
{
	enet_peer_send(m_peer, unreliable ? 1 : 0, enet_packet_create(data, len, unreliable ? ENET_PACKET_FLAG_UNSEQUENCED : ENET_PACKET_FLAG_RELIABLE));
}

void Instance::SendEncrypted(const std::vector<uint8_t>& data, bool unreliable)
{
	SendRaw(m_crypto->Encrypt(data), unreliable);
}

void Instance::SendEncrypted(const uint8_t* data, std::size_t len, bool unreliable)
{
	SendRaw(m_crypto->Encrypt(data, len), unreliable);
}

void Instance::SendEncMessage(uint32_t meta, const std::string& message)
{
	std::size_t len = sizeof(std::uint32_t) + message.length();
	std::uint8_t* data = new std::uint8_t[len];
	memcpy(data, &meta, sizeof(std::uint32_t));
	memcpy(data + sizeof(std::uint32_t), message.data(), message.length());
	SendEncrypted(data, len);
}

Crypto* Instance::GetCrypto()
{
	return m_crypto;
}
