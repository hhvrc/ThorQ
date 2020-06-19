#include "instance.h"

#include <iostream>

#include <enet.h>
#include <enums.h>
#include <crypto.h>

using namespace ThorQ;

Instance::Instance(ENetPeer* peer) : m_peer(peer)
{
	peer->data = this;
	m_crypto = new Crypto();
}

Instance::~Instance()
{
	delete m_crypto;
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

void Instance::SendEncMessage(uint32_t meta)
{
	meta = htonl(meta);
	SendEncrypted(reinterpret_cast<std::uint8_t*>(&meta), sizeof(std::uint32_t));
}

void Instance::SendEncMessage(uint32_t meta, const std::string& message)
{
	meta = htonl(meta);
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
