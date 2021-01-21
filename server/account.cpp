#include "account.h"

#include <set>
#include <memory>
#include <atomic>
#include <thread>
#include <future>

#include <fmt/core.h>
#include <schemas/user_generated.h>
#include <schemas/group_generated.h>
#include <schemas/account_generated.h>

#include "lsql/connection.h"
#include "lsql/transaction.h"
#include "lsql/column.h"
#include "lsql/query.h"

#include "instance.h"

std::shared_mutex g_accounts_lock;
std::unordered_map<std::string, std::shared_ptr<ThorQ::Account>> g_accounts;

ThorQ::Account::Account(std::int64_t dbId, const std::string& username, const std::string& passwordHash)
    : m_dbId(dbId)
    , m_username(username)
    , m_passwordHash(passwordHash)
    , m_sessions()
    , m_instances()
    , m_relationships()
{
}

std::shared_ptr<ThorQ::Account> ThorQ::Account::GetAccount(const std::string& username)
{
    {
        std::shared_lock l(g_accounts_lock);
        auto it = g_accounts.find(username);

        if (it != g_accounts.end())
        {
            return it->second;
        }
    }

    LSql::Connection connection("database.db", LSql::Connection::READWRITE);

    if (!connection.isOpen())
    {
        fmt::print(stderr, "SQL error: {}\n", connection.lastError());
        return nullptr;
    }

    LSql::Query query = connection.query("SELECT db_id, password_hash, authority FROM accounts WHERE username = ?");

    if (!query.bind(1, username))
    {
        fmt::print(stderr, "SQL error: {}\n", connection.lastError());
        return nullptr;
    }

    if (!query.step())
    {
        fmt::print(stderr, "SQL error: {}\n", connection.lastError());
        return nullptr;
    }

    if (query.columnCount() != 3)
    {
        fmt::print(stderr, "SQL error: {}\n", connection.lastError());
        return nullptr;
    }

    // Get database ID
    LSql::Column col = query.column(0);
    if (query.getType(0) != LSql::Type::Integer ||
        query.getType(1) != LSql::Type::Text    ||
        query.getType(2) != LSql::Type::Integer)
    {
        return nullptr;
    }

    // Get authority
    int authority = col.getInt();
    if (false)//authority < THORQ_ACCOUNT_AUTHORITY_NONE || authority > THORQ_ACCOUNT_AUTHORITY_FOUNDER)
    {
        return nullptr;
    }

    return std::shared_ptr<ThorQ::Account>(new ThorQ::Account(query.column(0).getInt64(), /*(THORQ_ACCOUNT_AUTHORITY)authority, */username, query.column(1).getText()));
}

std::shared_ptr<ThorQ::Account> ThorQ::Account::NewAccount(const std::string& username, const std::string& passwordHash)
{
    {
        std::shared_lock l(g_accounts_lock);
        auto it = g_accounts.find(username);

        if (it != g_accounts.end())
        {
            fmt::print("Account already exists: {}\n", username);
            return nullptr;
        }
    }

    LSql::Connection connection("database.db", LSql::Connection::READWRITE);

    if (!connection.isOpen())
    {
        return nullptr;
    }

    LSql::Transaction transaction = connection.transaction();

    if (!transaction.isOpen())
	{
        fmt::print(stderr, "SQL Failed to start transaction: {}\n", connection.lastError());
		return nullptr;
    }

    LSql::Query query = connection.query("INSERT OR IGNORE INTO accounts(username, password_hash) VALUES (?, ?);");

    if (!query.bind(1, username))
    {
        fmt::print(stderr, "SQL Failed to bind username: {}\n", connection.lastError());
        return nullptr;
    }

    if (!query.bind(2, passwordHash))
    {
        fmt::print(stderr, "SQL Failed to bind passwordHash: {}\n", connection.lastError());
        return nullptr;
    }

    if (!query.step())
    {
        fmt::print(stderr, "SQL Failed to execute account query: {}\n", connection.lastError());
        return nullptr;
    }

    std::int64_t i = connection.lastInsertedRowId();

    if (i == 0)
    {
        fmt::print(stderr, "username [{}] not available\n", username);
        return nullptr;
    }

    if (!transaction.commit())
	{
        fmt::print(stderr, "SQL Failed to commit account: {}\n", connection.lastError());
		return nullptr;
	}

    return std::shared_ptr<ThorQ::Account>(new ThorQ::Account(i, /*THORQ_ACCOUNT_AUTHORITY_NONE, */username, passwordHash));
}

int64_t ThorQ::Account::databaseId() const
{
    return m_dbId;
}

std::string ThorQ::Account::username() const
{
	return m_username;
}

bool ThorQ::Account::setUsername(const std::string& username)
{
    if (m_username == username)
    {
        return true;
    }

    if ( m_dbId <= 0)
    {
        return false;
    }

    LSql::Connection connection("database.db", LSql::Connection::READWRITE);

    if (!connection.isOpen())
    {
        return false;
    }

    LSql::Query query = connection.query("UPDATE OR IGNORE accounts SET username = ? WHERE db_id = ? LIMIT 1;SELECT changes();");

    if (!query.isValid())
    {
        fmt::print("Failed to create query: {}\n", connection.lastError());
        return false;
    }

    if (!query.bind(1, username))
    {
        fmt::print("Failed to bind username: {}\n", connection.lastError());
        return false;
    }

    if (!query.bind(2, m_dbId))
    {
        fmt::print("Failed to bind dbID: {}\n", connection.lastError());
        return false;
    }

    if (!query.step())
    {
        fmt::print("Failed to execute username query: {}\n", connection.lastError());
        return false;
    }

    if (query.columnCount() != 1)
    {
        fmt::print("Query didnt return any values\?\?\?\?\n");
        return false;
    }

    if (query.column(1).getInt() == 0)
    {
        fmt::print("account invalid/already used\n");
        return false;
    }

    std::unique_lock l(l_basics);
    m_username = username;
    return true;
}

std::string ThorQ::Account::passwordHash() const
{
    return m_passwordHash;
}
bool ThorQ::Account::setPasswordHash(const std::string& passwordHash)
{
    if (m_passwordHash == passwordHash)
    {
        return true;
    }

    if ( m_dbId <= 0)
    {
        return false;
    }

    LSql::Connection connection("database.db", LSql::Connection::READWRITE);

    if (!connection.isOpen())
    {
        return false;
    }

    LSql::Query query = connection.query("UPDATE OR IGNORE accounts SET password_hash = ? WHERE db_id = ? LIMIT 1;SELECT changes();");

    if (!query.isValid())
    {
        fmt::print("Failed to create query: {}\n", connection.lastError());
        return false;
    }

    if (!query.bind(1, passwordHash))
    {
        fmt::print("Failed to bind username: {}\n", connection.lastError());
        return false;
    }

    if (!query.bind(2, m_dbId))
    {
        fmt::print("Failed to bind dbID: {}\n", connection.lastError());
        return false;
    }

    if (!query.step())
    {
        fmt::print("Failed to execute username query: {}\n", connection.lastError());
        return false;
    }

    if (query.columnCount() != 1)
    {
        fmt::print("Query didnt return any values\?\?\?\?\n");
        return false;
    }

    if (query.column(1).getInt() == 0)
    {
        fmt::print("account invalid/already used\n");
        return false;
    }

    std::unique_lock l(l_basics);
    m_passwordHash = passwordHash;
    return true;
}

std::shared_ptr<ThorQ::Account> ThorQ::Account::master() const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_master));
    return m_master;
}

bool ThorQ::Account::isExclusive() const
{
    return m_exclusive;
}

bool ThorQ::Account::addRequestOutgoing(std::shared_ptr<ThorQ::Account> target)
{
    std::unique_lock l(l_requests);

    std::size_t sizeBefore = m_requests_outgoing.size();

    m_requests_outgoing.insert(target);

    return m_requests_outgoing.size() > sizeBefore;
}
bool ThorQ::Account::removeRequestOutgoing(std::shared_ptr<ThorQ::Account> target)
{
    std::unique_lock l(l_requests);
    return m_requests_outgoing.erase(target) > 0;
}
bool ThorQ::Account::containsRequestOutgoing(std::shared_ptr<ThorQ::Account> target) const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_requests));
    return m_requests_outgoing.contains(target);
}

bool ThorQ::Account::addRequestIncoming(std::shared_ptr<ThorQ::Account> source)
{
    std::unique_lock l(l_requests);

    std::size_t sizeBefore = m_requests_incoming.size();

    m_requests_incoming.insert(source);

    return m_requests_incoming.size() > sizeBefore;
}
bool ThorQ::Account::removeRequestIncoming(std::shared_ptr<ThorQ::Account> source)
{
    std::unique_lock l(l_requests);
    return m_requests_incoming.erase(source) > 0;
}
bool ThorQ::Account::containsRequestIncoming(std::shared_ptr<ThorQ::Account> source) const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_requests));
    return m_requests_incoming.contains(source);
}

void ThorQ::Account::requestSession(std::shared_ptr<ThorQ::Instance> source, std::shared_ptr<ThorQ::Account> targetAccount)
{
    std::vector<std::uint8_t> response;

    std::shared_ptr<ThorQ::Account> sourceAccount;
    {
        std::shared_lock l(source->l_account);
        sourceAccount = source->m_account;
    }

    // If account is logged out
    if (sourceAccount == nullptr)
    {
        // TODO: Protobuf response: [DENIED] Please log in
        return;
    }

    // If target account is self
    if (sourceAccount == targetAccount)
    {
        // TODO: Protobuf response: [DENIED] Cannot request on self
        return;
    }

    if (!sourceAccount->addRequestOutgoing(targetAccount))
    {
        // TODO: Protobuf response: [DENIED] Request already sent
        return;
    }

    // Ensure a record exists in receiver
    targetAccount->addRequestIncoming(sourceAccount);

    // TODO: Protobuf to {target} [Session][Event] Got request from {sourceAccount}

    // TODO: Protobuf response: [OK] Request sent
}

bool ThorQ::Account::requestAccept(std::shared_ptr<ThorQ::Account> sourceAccount, std::shared_ptr<ThorQ::Account> targetAccount)
{
    if (sourceAccount == targetAccount)
    {
        // TODO: Protobuf response: [DENIED] Cannot start session with self
        return false;
    }

    if (!sourceAccount->removeRequestOutgoing(targetAccount))
    {
        // Protobuf response: [DENIED] Request from {sourceAccount} is invalid / never got sent
        return false;
    }

    targetAccount->removeRequestIncoming(sourceAccount);

    // TODO: Insert a session containing the two

    // TODO: Protobuf to {source} [Session][Event] Session started {session}
    // TODO: Protobuf to {target} [Session][Event] Session started {session}

    // TODO: Protobuf response: [OK]

    return true;
}
bool ThorQ::Account::requestDeny(std::shared_ptr<ThorQ::Account> sourceAccount, std::shared_ptr<ThorQ::Account> targetAccount)
{
    if (sourceAccount == targetAccount)
    {
        // TODO: Protobuf response: [DENIED] Cannot deny session from self
        return false;
    }

    sourceAccount->removeRequestOutgoing(targetAccount);
    targetAccount->removeRequestIncoming(sourceAccount);

    // TODO: Protobuf response: [OK]

    return true;
}

bool ThorQ::Account::isOnline() const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_instances));
    return m_instances.size() > 0;
}

bool ThorQ::Account::addInstance(ThorQ::Instance* instance)
{
    std::unique_lock l(l_instances);
    return m_instances.insert(instance).second;
}

bool ThorQ::Account::removeInstance(ThorQ::Instance* instance)
{
    std::unique_lock l(l_instances);
    return m_instances.erase(instance) > 0;
}

bool ThorQ::Account::containsInstance(ThorQ::Instance* instance) const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_instances));
    return m_instances.contains(instance);
}

void ThorQ::Account::removeAllInstances()
{
    std::unique_lock l(l_instances);
    m_instances.clear();
}

void ThorQ::Account::setIsInSteamVR(bool value)
{
    if (isInSteamVR() != value)
    {
        if (value)
            m_activityState |= THORQ_USER_ACTIVITY_FLAG_OPENVR_RUNNING;
        else
            m_activityState &= ~THORQ_USER_ACTIVITY_FLAG_OPENVR_RUNNING;

        // TODO: Protobuf to friends: [Relationship][AccountState] activityState
    }
}

void ThorQ::Account::setHasCollar(bool value)
{
    if (hasCollar() != value)
    {
        if (value)
            m_activityState |= THORQ_USER_ACTIVITY_FLAG_COLLAR_PRESENT;
        else
            m_activityState &= ~THORQ_USER_ACTIVITY_FLAG_COLLAR_PRESENT;

        // TODO: Protobuf to friends: [Relationship][AccountState] activityState
    }
}


bool ThorQ::Account::isInSteamVR() const
{
    return (m_activityState & THORQ_USER_ACTIVITY_FLAG_OPENVR_RUNNING) != 0;
}

bool ThorQ::Account::hasCollar() const
{
    return (m_activityState & THORQ_USER_ACTIVITY_FLAG_COLLAR_PRESENT) != 0;
}
