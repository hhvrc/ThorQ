#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <atomic>
#include <memory>
#include <shared_mutex>
#include <unordered_set>
#include <set>
#include <vector>

#include <flatbuffers/flatbuffers.h>

#include <enums.h>

#include "typedefs_server.h"

namespace ThorQ {
class Account
{
    Account(std::int64_t dbId, const std::string& username, const std::string& passwordHash);
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

    bool addRequestOutgoing(std::shared_ptr<ThorQ::Account> target);
    bool removeRequestOutgoing(std::shared_ptr<ThorQ::Account> target);
    bool containsRequestOutgoing(std::shared_ptr<ThorQ::Account> target) const;

    bool addRequestIncoming(std::shared_ptr<ThorQ::Account> source);
    bool removeRequestIncoming(std::shared_ptr<ThorQ::Account> source);
    bool containsRequestIncoming(std::shared_ptr<ThorQ::Account> source) const;

    static void requestSession(std::shared_ptr<ThorQ::ApiServerConnection> source, std::shared_ptr<ThorQ::Account> target);
    static bool requestAccept(std::shared_ptr<ThorQ::Account> sender, std::shared_ptr<ThorQ::Account> target);
    static bool requestDeny(std::shared_ptr<ThorQ::Account> sender, std::shared_ptr<ThorQ::Account> target);

    bool isOnline() const;
    bool addInstance(std::shared_ptr<ThorQ::ApiServerConnection> instance);
    bool removeInstance(std::shared_ptr<ThorQ::ApiServerConnection> instance);
    bool containsInstance(std::shared_ptr<ThorQ::ApiServerConnection> instance) const;
    void removeAllInstances();

    void setIsInSteamVR(bool hasCollar);
    void setHasCollar(bool hasCollar);

    bool isInSteamVR() const;
    bool hasCollar() const;

    void ban();
private:
    const std::int64_t m_dbId;

    std::shared_mutex l_basics;
    std::string m_username;
    std::string m_passwordHash;

    std::uint16_t m_activityState; // enum: thorq_user_activity_flag

    std::shared_mutex l_master;
    std::shared_ptr<ThorQ::Account> m_master; // This persons master
    std::atomic_bool m_exclusive;             // This person is exclusive to their master

    std::shared_mutex l_requests;
    std::set<std::shared_ptr<ThorQ::Account>> m_requests_incoming;
    std::set<std::shared_ptr<ThorQ::Account>> m_requests_outgoing;

    std::shared_mutex l_sessions;
    std::set<std::shared_ptr<ThorQ::Group>> m_sessions;

    std::shared_mutex l_instances;
    std::set<std::shared_ptr<ThorQ::ApiServerConnection>> m_instances;

    std::shared_mutex l_relationships;
    std::set<std::shared_ptr<ThorQ::Relationship>> m_relationships;
};
}

#endif // ACCOUNT_H
