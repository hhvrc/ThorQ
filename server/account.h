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
class Account
{
    Account(std::int64_t dbId, THORQ_ACCOUNT_AUTHORITY authority, const char* username, const char* passwordHash);
public:
    static Account* GetAccount(const char* username);
    static Account* NewAccount(const char* username, const char* password);
public:
    const std::string& username() const;
    void setUsername(const std::string& username);

    void setPassword(const std::string& password);
    void verifyPassword(const std::string& password) const;

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

    void setStatus(std::uint16_t flags);
    std::uint16_t status();

    bool isInSession() const;
    bool isInSteamVR() const;
    bool hasCollar() const;

    void ban();
    void fuckYou();
    void disconnectPeers();
    void sendMessage(const std::vector<std::uint8_t>& message, bool encrypt = true, bool reliable = true);
    void sendMessageToFriends(const std::vector<std::uint8_t>& message, bool encrypt = true, bool reliable = true);
private:
    void onPasswordHashingDone(const std::string& hash);
    void onPasswordVerificationDone(bool result);

    std::int64_t m_dbId;

    QReadWriteLock l_basics;
    std::string m_username;
    std::string m_passwordHash;

    std::uint16_t m_activityState; // enum: thorq_user_activity_flag

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
