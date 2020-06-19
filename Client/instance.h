#ifndef INSTANCE_H
#define INSTANCE_H

#include <string>
#include <memory>
#include <vector>
#include <cstdint>

typedef struct _ENetPeer ENetPeer;

namespace ThorQ {

	class Crypto;

	class Instance
	{
		ENetPeer* m_peer;

		Crypto* m_crypto;

		Instance(const Instance&) = delete;
		Instance& operator=(const Instance&) = delete;
	public:
		Instance(ENetPeer* peer);
		Instance(ENetPeer* peer, const std::string& name);
		~Instance();

		void SetPeer(ENetPeer* peer);
		ENetPeer* Peer() const;

		void SendRaw(const std::vector<std::uint8_t>& data, bool unreliable = false);
		void SendRaw(const std::uint8_t* data, std::size_t len, bool unreliable = false);
		void SendEncrypted(const std::vector<std::uint8_t>& data, bool unreliable = false);
		void SendEncrypted(const std::uint8_t* data, std::size_t len, bool unreliable = false);
		void SendEncMessage(std::uint32_t meta);
		void SendEncMessage(std::uint32_t meta, const std::string& message);

		Crypto* GetCrypto();
	};
}

#endif // INSTANCE_H
