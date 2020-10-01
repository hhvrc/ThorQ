#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QObject>
#include <QUuid>
#include <QSet>

#include <typedefs.h>

#include "relationship.h"

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

    QSet<Account*> friends();
    QSet<Instance*> instances();

    void requestOn(Instance* target);
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
	void usernameChanged(const QString& username);
private:
	int m_dbId;
	QUuid uuid; // Constant, Is never sent to a client

	QString m_username;
	QString m_passwordHash;

	bool m_isAdmin;

    QSet<Account*> m_friends;
    QSet<Instance*> m_instances;
    QSet<Relationship*> m_relationships;
};
}

#endif // ACCOUNT_H
