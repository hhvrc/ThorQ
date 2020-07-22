#ifndef INSTANCE_H
#define INSTANCE_H

#include <string>
#include <memory>
#include <vector>
#include <cstdint>

#include <enums.h>

typedef struct _ENetPeer ENetPeer;

namespace ThorQ {

	class Crypto;

	class Instance
	{
		Instance(const Instance&) = delete;
		Instance& operator=(const Instance&) = delete;
	public:
		Instance(ENetPeer* peer);
		Instance(ENetPeer* peer, const std::string& name);
		~Instance();

		void SetName(const std::string& newName);
		const std::string& Name() const;
		bool HasName();

		void SetPeer(ENetPeer* peer);
		ENetPeer* Peer() const;

		void RequestOn(Instance* target);
		bool RequestAcceptFrom(Instance* sender);
		bool RequestDenyFrom(Instance* sender);
		Instance* Partner() const;
		void ClearPartner();
		bool HasPartner();

		void SetHasCollar(bool hasCollar);
		bool HasCollar() const;

		thorq_connection_state_t ConnectionState() const;
		void SetConnectionState(thorq_connection_state_t state);
		thorq_crypto_state_t CryptoState() const;
		void SetCryptoState(thorq_crypto_state_t state);
		thorq_login_state_t LoginState() const;
		void SetLoginState(thorq_login_state_t state);
		thorq_session_state_t SessionState() const;
		void SetSessionState(thorq_session_state_t state);

		void SendHeartbeat();
		void SendRaw(std::uint32_t meta);
		void SendRaw(std::uint32_t meta, const std::string& message, bool unreliable = false);
		void SendRaw(const std::vector<std::uint8_t>& data, bool unreliable = false);
		void SendRaw(const std::uint8_t* data, std::size_t len, bool unreliable = false);
		void SendEncrypted(std::uint32_t meta);
		void SendEncrypted(std::uint32_t meta, const std::string& message, bool unreliable = false);
		void SendEncrypted(const std::vector<std::uint8_t>& data, bool unreliable = false);
		void SendEncrypted(const std::uint8_t* data, std::size_t len, bool unreliable = false);

		void CryptoInit();
		bool CryptoEstablish(const std::vector<std::uint8_t>& data);
		bool CryptoVerify(const std::vector<std::uint8_t>& data);

		Crypto* GetCrypto();
	private:
		std::uint8_t GetFlag(bool withHeartbeat = false);

		Crypto* m_crypto;

		thorq_connection_state_t m_connectionState;
		thorq_crypto_state_t m_cryptoState;
		thorq_login_state_t m_loginState;
		thorq_session_state_t m_sessionState;

		std::string m_name;
		bool m_hasCollar;

		ENetPeer* m_peer;
		Instance* m_partner;
		Instance* m_requestedPartner;

		std::uint8_t m_verificationData[256];
	};
}

#endif // INSTANCE_H
