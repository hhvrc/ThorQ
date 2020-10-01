#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <memory>
#include <unordered_set>

#include <QObject>
#include <QUuid>
#include <QSet>
#include <QReadWriteLock>

#include <enums.h>
#include "typedefs_server.h"

namespace ThorQ {
class Account : public QObject
{
	Q_OBJECT

	Account(QObject* parent = nullptr);
public:
	static Account* GetAccount(const QString& username);
	static Account* NewAccount(const QString& username, const QString& password, const QString& registrationKey);
public:
	const QString& username() const;
	void setUsername(const QString& username);

    void setPassword(const QString& password);
	bool verifyPassword(const QString& password);

    QSet<Session*> sessions();
    QSet<Instance*> instances();
    QSet<Relationship*> relationships();

    static void requestSession(Account* sender, Account* receiver);
    bool requestAcceptFrom(Instance* sender);
    bool requestDenyFrom(Instance* sender);
    Instance* partner() const;

    void setIsInSteamVR(bool hasCollar);
    void setHasCollar(bool hasCollar);

    THORQ_STATE_SESSION sessionState() const;
    void setSessionState(THORQ_STATE_SESSION state);
    void setActivityState(quint8 state);
    quint8 activityState() const;

    bool isInSession() const;
    bool isInSteamVR() const;
    bool hasCollar() const;
signals:
    void isOnlineChanged(bool isOnline);
    void statusChanged();
	void usernameChanged(const QString& username);
public slots:
    void ban();
    void fuckYou();
    void disconnectPeers();
    void sendMessage(const std::vector<std::uint8_t>& message, bool encrypt = true, bool reliable = true);
    void sendMessageToFriends(const std::vector<std::uint8_t>& message, bool encrypt = true, bool reliable = true);
private:
    int m_dbId;

	QString m_username;
	QString m_passwordHash;

    THORQ_STATE_SESSION m_sessionState;
    THORQ_ACCOUNT_AUTHORITY m_authority;

    Account* m_master; // This persons master
    bool m_exclusive;  // This person is exclusive to their master

    QReadWriteLock l_sessions;
    QSet<Session*> m_sessions;

    QReadWriteLock  l_instances;
    QSet<Instance*> m_instances;

    QReadWriteLock      l_relationships;
    QSet<Relationship*> m_relationships;
};
}

#endif // ACCOUNT_H
