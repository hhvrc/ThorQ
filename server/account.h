#ifndef ACCOUNT_H
#define ACCOUNT_H


#include "typedefs_server.h"

#include <uuid.h>
#include <enums.h>
#include <cryptography/hashing.h>

#include <flatbuffers/flatbuffers.h>

#include <atomic>
#include <shared_mutex>
#include <unordered_set>
#include <vector>
#include <set>
#include <memory>

namespace ThorQ {
class Account
{
    Account(std::int64_t dbId, ThorQ::Uuid id, const std::string& username);
public:
    static std::shared_ptr<ThorQ::Account> GetAccount(const std::string& username);
    inline static std::shared_ptr<ThorQ::Account> GetAccount(const flatbuffers::String* username) { return GetAccount(std::string(username->data(), username->size())); }
    static std::shared_ptr<ThorQ::Account> NewAccount(const std::string& username);
    inline static std::shared_ptr<ThorQ::Account> NewAccount(const flatbuffers::String* username) { return NewAccount(std::string(username->data(), username->size())); }
public:
    ThorQ::Uuid id() const { return m_id; }
    std::int64_t dbId() const { return m_dbId; }

    std::string username() const;
    bool setUsername(const std::string& username);

    ThorQ::Crypto::Hashing::CalculatedHash passwordHash() const;
    bool setPasswordHash(ThorQ::Crypto::Hashing::CalculatedHash hash);
    ThorQ::Crypto::Hashing::HashingParameters passwordHashParameters() const;
    bool setPasswordHash(ThorQ::Crypto::Hashing::HashingParameters params);

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
    const ThorQ::Uuid m_id;
    const std::int64_t m_dbId;

    std::shared_mutex l_basics;
    std::string m_username;
    ThorQ::Crypto::Hashing::CalculatedHash m_passwordHash;
    ThorQ::Crypto::Hashing::HashingParameters m_passwordHashParameters;

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
