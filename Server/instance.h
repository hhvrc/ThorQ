#ifndef INSTANCE_H
#define INSTANCE_H

#include <string>
#include <vector>
#include <cstdint>

#include <enums.h>
#include <constants.h>

#include "typedefs.h"

namespace ThorQ {
class Instance
{
	Instance(const Instance&) = delete;
	Instance& operator=(const Instance&) = delete;
public:
	Instance(ENetPeer* peer);
	Instance(ENetPeer* peer, const std::string& name);
	~Instance();

	void setName(const std::string& newName);
	const std::string& name() const;
	bool hasName();

	void setPeer(ENetPeer* peer);
	ENetPeer* peer() const;

	void requestOn(Instance* target);
	bool requestAcceptFrom(Instance* sender);
	bool requestDenyFrom(Instance* sender);
	Instance* partner() const;
	void clearPartner();
	bool hasPartner();

	void setHasCollar(bool hasCollar);
	bool hasCollar() const;

	thorq_connection_state_t connectionState() const;
	void setConnectionState(thorq_connection_state_t state);
	thorq_crypto_state_t cryptoState() const;
	void setCryptoState(thorq_crypto_state_t state);
	thorq_auth_state_t authState() const;
	void setAuthState(thorq_auth_state_t state);
	thorq_login_state_t loginState() const;
	void setLoginState(thorq_login_state_t state);
	thorq_session_state_t sessionState() const;
	void setSessionState(thorq_session_state_t state);

	void cryptoInit();
	bool cryptoEstablish(const std::vector<std::uint8_t>& data);
	bool cryptoVerify(const std::vector<std::uint8_t>& data);

	Crypto* getCrypto();

	void sendPayload(const thorq_payload_t* payload, bool encrypt = true, bool reliable = true);
	void sendMessage(const std::vector<std::uint8_t>& message, bool encrypt = true, bool reliable = true);

	void disconnect(std::uint32_t reason);
	void disconnectForcibly(std::uint32_t reason);
private:
	Crypto* m_crypto;

	thorq_connection_state_t m_connectionState;
	thorq_crypto_state_t m_cryptoState;
	thorq_auth_state_t m_authState;
	thorq_login_state_t m_loginState;
	thorq_session_state_t m_sessionState;

	std::string m_name;
	bool m_hasCollar;

	ENetPeer* m_peer;
	Instance* m_partner;
	Instance* m_requestedPartner;

	std::uint8_t m_verificationData[THORQ_CRYPTO_VERIFICATION_DATA_LENGTH];
};
}

#endif // INSTANCE_H
