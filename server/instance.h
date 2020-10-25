#ifndef INSTANCE_H
#define INSTANCE_H

#include <cstdint>
#include <vector>

#include <memory>
#include <enums.h>
#include <constants.h>
#include <typedefs_global.h>
#include <flatbuffers/flatbuffers.h>

#include "typedefs_server.h"

namespace ThorQ {
class Instance
{
    Instance(const Instance&) = delete;
    Instance& operator=(const Instance&) = delete;
public:
    Instance(ENetPeer* peer);
    Instance(ENetPeer* peer, const std::string& name);
    ~Instance();

    void setAccount(std::shared_ptr<ThorQ::Account> account);
    std::shared_ptr<ThorQ::Account> account() const;

    void setHwid(const std::vector<std::uint8_t>& hwid);
    std::vector<std::uint8_t> hwid() const;

	void setPeer(ENetPeer* peer);
    ENetPeer* peer() const;

	THORQ_STATE_CRYPTO cryptoState() const;
	void setCryptoState(THORQ_STATE_CRYPTO state);
	THORQ_STATE_AUTH authState() const;
    void setAuthState(THORQ_STATE_AUTH state);

	void cryptoInit();
    bool cryptoEstablish(const flatbuffers::Vector<std::uint8_t>& data);
    bool cryptoVerify(const flatbuffers::Vector<std::uint8_t>& data);

	Crypto* getCrypto();

    void sendPayload(const flatbuffers::DetachedBuffer& payload, THORQ_CHANNEL ch, bool encrypt = true, bool reliable = true);

    void disconnectPeer(std::uint32_t reason);
    void disconnectPeerForcibly(std::uint32_t reason);
private:
    ENetPeer* m_peer;
    std::shared_ptr<ThorQ::Account> m_account;

    ThorQ::Crypto* m_crypto;
    std::uint8_t*  m_verificationData;

    std::vector<std::uint8_t> m_systemID;

    THORQ_STATE_CRYPTO m_cryptoState;
    THORQ_STATE_AUTH   m_authState;
};
}

#endif // INSTANCE_H
