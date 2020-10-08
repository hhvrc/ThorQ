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
    static Account* NewAccount(const QString& username, const QString& password);
public:
	const QString& username() const;
	void setUsername(const QString& username);

    void setPassword(const QString& password);
    void verifyPassword(const QString& password) const;

    Account* master() const;
    bool isExclusive() const;


    QSet<Session*> sessions() const;
    QSet<Instance*> instances() const;
    QSet<Relationship*> relationships() const;

    static void requestSession(Instance* source, Account* target);
    bool requestAcceptFrom(Account* sender);
    bool requestDenyFrom(Account* sender);

    void setIsInSteamVR(bool hasCollar);
    void setHasCollar(bool hasCollar);

    std::uint16_t status();

    bool isInSession() const;
    bool isInSteamVR() const;
    bool hasCollar() const;
signals:
    void usernameChanged(const QString& username);
    void masterChanged(const Account* master);
    void isExclusiveChanged(bool isExclusive);
    void isOnlineChanged(bool isOnline);
    void statusChanged();
public slots:
    void setStatus(std::uint16_t flags);
    void ban();
    void fuckYou();
    void disconnectPeers();
    void sendMessage(const std::vector<std::uint8_t>& message, bool encrypt = true, bool reliable = true);
    void sendMessageToFriends(const std::vector<std::uint8_t>& message, bool encrypt = true, bool reliable = true);
private slots:
    void onPasswordHashingDone(const std::string& hash);
    void onPasswordVerificationDone(bool result);
private:
    int m_dbId;
    QUuid m_publicId;

    QReadWriteLock l_basics;
	QString m_username;
	QString m_passwordHash;

    quint8 m_activityState; // enum: thorq_user_activity_flag

    THORQ_STATE_SESSION m_sessionState;
    THORQ_ACCOUNT_AUTHORITY m_authority;

    QReadWriteLock l_master;
    Account* m_master;    // This persons master
    std::atomic<bool> m_exclusive; // This person is exclusive to their master

    QReadWriteLock  l_requests;
    QSet<Instance*> m_requests_incoming;
    QSet<Instance*> m_requests_outgoing;

    QReadWriteLock l_sessions;
    QSet<Session*> m_sessions;

    QReadWriteLock  l_instances;
    QSet<Instance*> m_instances;

    QReadWriteLock      l_relationships;
    QSet<Relationship*> m_relationships;
};
}

#endif // ACCOUNT_H
