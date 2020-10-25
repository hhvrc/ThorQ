#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <atomic>
#include <memory>
#include <shared_mutex>
#include <unordered_set>
#include <tbb/concurrent_unordered_set.h>

#include <enums.h>
#include "typedefs_server.h"

namespace ThorQ {
class Account
{
    friend ThorQ::Instance;
    Account(std::int64_t dbId, THORQ_ACCOUNT_AUTHORITY authority, const std::string& username, const std::string& passwordHash);
public:
    static std::shared_ptr<ThorQ::Account> GetAccount(const std::string& username);
    static std::shared_ptr<ThorQ::Account> NewAccount(const std::string& username, const std::string& passwordHash);
public:
    std::int64_t databaseId() const;

    std::string username() const;
    bool setUsername(const std::string& username);

    std::string passwordHash() const;
    bool setPasswordHash(const std::string& passwordHash);

    std::shared_ptr<ThorQ::Account> master() const;
    bool isExclusive() const;

    static void requestSession(std::shared_ptr<ThorQ::Instance> source, std::shared_ptr<ThorQ::Account> target);
    bool requestAcceptFrom(std::shared_ptr<ThorQ::Account> sender);
    bool requestDenyFrom(std::shared_ptr<ThorQ::Account> sender);

    void setIsInSteamVR(bool hasCollar);
    void setHasCollar(bool hasCollar);

    void setStatus(std::uint16_t flags);
    std::uint16_t status();

    bool isInSession() const;
    bool isInSteamVR() const;
    bool hasCollar() const;

    void ban();
    void fuckYou();
    void disconnectAllInstances();
    void sendPayload(const std::vector<std::uint8_t>& payload, THORQ_CHANNEL ch, bool encrypt = true, bool reliable = true);
    void sendPayloadToFriends(const std::vector<std::uint8_t>& payload, THORQ_CHANNEL ch, bool encrypt = true, bool reliable = true);
private:
    const std::int64_t m_dbId;

    std::shared_mutex l_basics;
    std::string m_username;
    std::string m_passwordHash;

    std::uint16_t m_activityState; // enum: thorq_user_activity_flag

    THORQ_ACCOUNT_AUTHORITY m_authority;

    std::shared_mutex l_master;
    std::shared_ptr<ThorQ::Account> m_master; // This persons master
    std::atomic_bool m_exclusive;             // This person is exclusive to their master

    tbb::concurrent_unordered_set<std::shared_ptr<ThorQ::Instance>> m_requests_incoming;
    tbb::concurrent_unordered_set<std::shared_ptr<ThorQ::Instance>> m_requests_outgoing;

    tbb::concurrent_unordered_set<std::shared_ptr<ThorQ::Session>> m_sessions;
    tbb::concurrent_unordered_set<std::shared_ptr<ThorQ::Instance>> m_instances;
    tbb::concurrent_unordered_set<std::shared_ptr<ThorQ::Relationship>> m_relationships;
};
}

#endif // ACCOUNT_H
