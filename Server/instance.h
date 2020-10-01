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
class Instance : public QObject
{
	Q_OBJECT

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
	THORQ_STATE_LOGIN loginState() const;
    void setLoginState(THORQ_STATE_LOGIN state);

	void cryptoInit();
	bool cryptoEstablish(const std::vector<std::uint8_t>& data);
	bool cryptoVerify(const std::vector<std::uint8_t>& data);

	Crypto* getCrypto();

	void sendMessage(std::vector<std::uint8_t>& message, bool encrypt = true, bool reliable = true);
    void sendMessage(const std::vector<std::uint8_t>& message, bool encrypt = true, bool reliable = true);
    void sendRaw(const std::vector<std::uint8_t>& raw, bool reliable = true);

    void disconnectPeer(quint32 reason);
    void disconnectPeerForcibly(quint32 reason);
signals:
    void disconnecting();
private:
	QUuid m_id;

    quint8 m_activityState; // enum: thorq_user_activity_flag

    THORQ_STATE_CRYPTO     m_cryptoState;
    THORQ_STATE_AUTH       m_authState;
    THORQ_STATE_LOGIN      m_loginState;

	ENetPeer* m_peer;
	QByteArray m_hwid;
	Account* m_account;

    Instance* m_partner;

    QSet<Instance*> m_incoming_requests;
    QSet<Instance*> m_outgoing_requests;

	Crypto* m_crypto;
    quint8 m_verificationData[THORQ_CRYPTO_VERIFICATION_DATA_LENGTH];
};
}

#endif // INSTANCE_H
