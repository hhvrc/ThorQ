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

        int ClientState() const;
        void SetClientState(int state);

        int CryptoState() const;
        void SetCryptoState(int state);

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
		void CryptoEstablish(const std::uint8_t* data, std::size_t size);
		bool CryptoVerify(const std::uint8_t* data, std::size_t size);

        Crypto* GetCrypto();
    private:
        std::uint8_t GetFlag(bool withHeartbeat = false);

        Crypto* m_crypto;

        int m_clientState;
        int m_cryptoState;

        std::string m_name;
        bool m_hasCollar;

        ENetPeer* m_peer;
        Instance* m_partner;
        Instance* m_requestedPartner;

		std::uint8_t m_verificationData[256];
	};
}

#endif // INSTANCE_H
