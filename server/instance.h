#ifndef INSTANCE_H
#define INSTANCE_H

#include <QSet>
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QUuid>
#include <cstdint>

#include <enums.h>
#include <constants.h>
#include <typedefs_global.h>

#include "typedefs_server.h"

namespace ThorQ {
class Instance
{
    Instance(const Instance&) = delete;
    Instance& operator=(const Instance&) = delete;
public:
    Instance(ENetPeer* peer, QObject* parent = nullptr);
    Instance(ENetPeer* peer, const std::string& name);
    ~Instance();

	void setAccount(Account* account);
	Account* account() const;

	void setHwid(const QByteArray& hwid);
    const QByteArray& hwid() const;

	void setPeer(ENetPeer* peer);
    ENetPeer* peer() const;

	THORQ_STATE_CRYPTO cryptoState() const;
	void setCryptoState(THORQ_STATE_CRYPTO state);
	THORQ_STATE_AUTH authState() const;
    void setAuthState(THORQ_STATE_AUTH state);

	void cryptoInit();
	bool cryptoEstablish(const std::vector<std::uint8_t>& data);
	bool cryptoVerify(const std::vector<std::uint8_t>& data);

	Crypto* getCrypto();

    void sendMessage(std::vector<std::uint8_t>& message, THORQ_CHANNEL ch, bool encrypt = true, bool reliable = true);
    void sendMessage(const std::vector<std::uint8_t>& message, THORQ_CHANNEL ch, bool encrypt = true, bool reliable = true);
    void sendRaw(const std::vector<std::uint8_t>& raw, THORQ_CHANNEL ch, bool reliable = true);

    void disconnectPeer(quint32 reason);
    void disconnectPeerForcibly(quint32 reason);
signals:
    void disconnecting();
private:
    ENetPeer* m_peer;
    ThorQ::Account* m_account;

    ThorQ::Crypto* m_crypto;
    std::uint8_t*  m_verificationData;

    QByteArray m_systemID;

    THORQ_STATE_CRYPTO m_cryptoState;
    THORQ_STATE_AUTH   m_authState;
};
}

#endif // INSTANCE_H
