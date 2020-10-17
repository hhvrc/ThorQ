#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <atomic>
#include <memory>
#include <shared_mutex>
#include <unordered_set>

#include <enums.h>
#include "typedefs_server.h"

namespace ThorQ {
class Account
{
    Account(std::int64_t dbId, THORQ_ACCOUNT_AUTHORITY authority, const std::string& username, const std::string& passwordHash);
public:
    static Account* GetAccount(const std::string& username);
    static Account* NewAccount(const std::string& username, const std::string& passwordHash);
public:
    std::string username() const;
    bool setUsername(const std::string& username);

    std::string passwordHash() const;
    bool setPasswordHash(const std::string& passwordHash);

    Account* master() const;
    bool isExclusive() const;

    std::unordered_set<Session*> sessions() const;
    std::unordered_set<Instance*> instances() const;
    std::unordered_set<Relationship*> relationships() const;

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
    std::int64_t m_dbId;

    std::shared_mutex l_basics;
    std::string m_username;
    std::string m_passwordHash;

    std::uint16_t m_activityState; // enum: thorq_user_activity_flag

    THORQ_ACCOUNT_AUTHORITY m_authority;

    std::shared_mutex l_master;
    Account* m_master;    // This persons master
    std::atomic<bool> m_exclusive; // This person is exclusive to their master

    std::shared_mutex l_requests;
    std::unordered_set<Instance*> m_requests_incoming;
    std::unordered_set<Instance*> m_requests_outgoing;

    std::shared_mutex l_sessions;
    std::unordered_set<Session*> m_sessions;

    std::shared_mutex l_instances;
    std::unordered_set<Instance*> m_instances;

    std::shared_mutex l_relationships;
    std::unordered_set<Relationship*> m_relationships;
};
}

#endif // ACCOUNT_H
