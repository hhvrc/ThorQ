#ifndef INSTANCE_H
#define INSTANCE_H

#include <cstdint>
#include <vector>
#include <memory>
#include <shared_mutex>

#include <enums.h>
#include <constants.h>
#include <typedefs_global.h>
#include <flatbuffers/flatbuffers.h>

#include "typedefs_server.h"

namespace ThorQ {
class Instance
{
    friend ThorQ::Server;
    Instance(const Instance&) = delete;
    Instance& operator=(const Instance&) = delete;
public:
    Instance(ENetPeer* peer);
    ~Instance();

    std::shared_ptr<ThorQ::Account> account() const;
    void accountSwap(std::shared_ptr<ThorQ::Account>& account);

    void setHwid(const std::vector<std::uint8_t>& hwid);
    std::vector<std::uint8_t> hwid() const;

    void setPeer(ENetPeer* peer);
    ENetPeer* peer() const;

	THORQ_STATE_CRYPTO cryptoState() const;
	void setCryptoState(THORQ_STATE_CRYPTO state);
    THORQ_STATE_HWID hwidState() const;
    void setHwidState(THORQ_STATE_HWID state);

    bool cryptoInit();
    bool cryptoEstablish(const flatbuffers::Vector<std::uint8_t>& data);
    bool cryptoVerify(const flatbuffers::Vector<std::uint8_t>& data);

    void packetSend(const flatbuffers::DetachedBuffer& payload, THORQ_CHANNEL ch, bool encrypt = true, bool reliable = true);
    bool packetDecode();

    void disconnectPeer(THORQ_DISCONNECT_REASON reason, bool force = false);
private:
    ENetPeer* m_peer;

    std::shared_mutex l_account;
    std::shared_ptr<ThorQ::Account> m_account;

    std::shared_mutex l_crypto;
    std::shared_ptr<ThorQ::Crypto> m_crypto;

    std::vector<std::uint8_t> m_verificationData;

    std::vector<std::uint8_t> m_systemID;

    THORQ_STATE_CRYPTO m_cryptoState;
    THORQ_STATE_HWID   m_hwidState;
};
}

#endif // INSTANCE_H
