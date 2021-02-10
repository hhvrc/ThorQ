#include "account.h"

#include <schemas_common.h>

#include <lsql/connection.h>
#include <lsql/transaction.h>
#include <lsql/column.h>
#include <lsql/query.h>
#include <fmt/core.h>

#include <future>
#include <set>
#include <thread>
#include <memory>
#include <atomic>

#include "apiserver_connection.h"

using namespace std::literals;

std::shared_mutex g_accounts_lock;
std::unordered_map<std::string, std::shared_ptr<ThorQ::Account>> g_accounts;

ThorQ::Account::Account(std::int64_t dbId, ThorQ::Uuid id, const std::string& username)
    : m_id(id)
    , m_dbId(dbId)
    , l_basics()
    , m_username(username)
    , m_passwordHash()
    , m_passwordHashParameters()
    , m_activityState(0)
    , l_master()
    , m_master()
    , m_exclusive()
    , l_requests()
    , m_requests_incoming()
    , m_requests_outgoing()
    , l_sessions()
    , m_sessions()
    , l_instances()
    , m_instances()
    , l_relationships()
    , m_relationships()
{
}

std::shared_ptr<ThorQ::Account> ThorQ::Account::GetAccount(const std::string& username)
{
    {
        std::shared_lock l(g_accounts_lock);
        auto it = g_accounts.find(username);

        if (it != g_accounts.end()) {
            return it->second;
        }
    }

    auto dbConnection = SQLite::Connection::OpenConnection("database.db", SQLite::Connection::READONLY);

    if (dbConnection == nullptr) {
        fmt::print(stderr, "Failed to open database\n");
        return nullptr;
    }

    // TODO remove "NOT NULL" on all other fields
    SQLite::Query query = dbConnection->makeQuery("SELECT db_id, uuid FROM accounts WHERE username = ?"sv);

    if (!query.bindText(1, username)) {
        fmt::print(stderr, "SQL error: {}\n", dbConnection->lastError());
        return nullptr;
    }

    if (!query.step()) {
        fmt::print(stderr, "SQL error: {}\n", dbConnection->lastError());
        return nullptr;
    }

    if (query.columnCount() != 2) {
        fmt::print(stderr, "SQL error: {}\n", dbConnection->lastError());
        return nullptr;
    }

    // Get database ID
    SQLite::Column dbIdCol = query.column(0);
    SQLite::Column uuidCol = query.column(1);
    if (dbIdCol.type() != SQLite::Type::Integer || uuidCol.type() != SQLite::Type::Text) {
        return nullptr;
    }

    std::int64_t dbId = dbIdCol.getInt64();

    ThorQ::Uuid accountId;
    if (!ThorQ::Uuid::TryParse(dbIdCol.getDataText(), accountId)) {
        return nullptr;
    }

    auto account = std::shared_ptr<ThorQ::Account>(new ThorQ::Account(dbId, accountId, username));

    // TODO fill in the rest

    return account;
}

std::shared_ptr<ThorQ::Account> ThorQ::Account::NewAccount(const std::string& username)
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

    auto dbConnection = SQLite::Connection::OpenConnection("database.db", SQLite::Connection::READONLY);

    if (dbConnection == nullptr) {
        fmt::print(stderr, "Failed to open database\n");
        return nullptr;
    }

    auto transaction = dbConnection->beginDeferredTransaction();

    if (!transaction.isOpen())
	{
        fmt::print(stderr, "SQL Failed to start transaction: {}\n", dbConnection->lastError());
		return nullptr;
    }

    SQLite::Query query = dbConnection->makeQuery("INSERT OR IGNORE INTO accounts(uuid, username) VALUES (?, ?)"sv);

    ThorQ::Uuid id = ThorQ::Uuid::NewUuid();
    if (!query.bindText(1, id.toString()))
    {
        fmt::print(stderr, "SQL Failed to bind username: {}\n", dbConnection->lastError());
        return nullptr;
    }

    if (!query.bindText(2, username))
    {
        fmt::print(stderr, "SQL Failed to bind username: {}\n", dbConnection->lastError());
        return nullptr;
    }

    if (!query.step())
    {
        fmt::print(stderr, "SQL Failed to execute account query: {}\n", dbConnection->lastError());
        return nullptr;
    }

    std::int64_t dbId = dbConnection->lastInsertedRowId();

    /*
    if (dbId == 0)
    {
        fmt::print(stderr, "username [{}] not available\n", username);
        return nullptr;
    }*/

    if (!transaction.commit())
	{
        fmt::print(stderr, "SQL Failed to commit account: {}\n", dbConnection->lastError());
		return nullptr;
	}

    return std::shared_ptr<ThorQ::Account>(new ThorQ::Account(dbId, id, username));
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

    if (m_dbId <= 0)
    {
        return false;
    }

    auto dbConnection = SQLite::Connection::OpenConnection("database.db", SQLite::Connection::READWRITE);

    if (dbConnection == nullptr) {
        fmt::print(stderr, "Failed to open database\n");
        return false;
    }

    auto transaction = dbConnection->beginDeferredTransaction();

    if (!transaction.isOpen())
    {
        fmt::print(stderr, "SQL Failed to start transaction: {}\n", dbConnection->lastError());
        return false;
    }

    SQLite::Query query = dbConnection->makeQuery("UPDATE OR IGNORE accounts SET username = ? WHERE db_id = ? LIMIT 1"sv);

    if (!query.isValid())
    {
        fmt::print("Failed to create query: {}\n", dbConnection->lastError());
        return false;
    }

    if (!query.bindText(1, username))
    {
        fmt::print("Failed to bind username: {}\n", dbConnection->lastError());
        return false;
    }

    if (!query.bindInt64(2, m_dbId))
    {
        fmt::print("Failed to bind dbID: {}\n", dbConnection->lastError());
        return false;
    }

    if (!query.step())
    {
        fmt::print("Failed to execute username query: {}\n", dbConnection->lastError());
        return false;
    }

    auto getChanges = dbConnection->makeQuery("SELECT changes();"sv);

    if (getChanges.columnCount() != 1)
    {
        fmt::print("Query didnt return any values\?\?\?\?\n");
        return false;
    }

    if (getChanges.column(1).getInt64() == 0)
    {
        fmt::print("account invalid/already used\n");
        return false;
    }

    transaction.commit();

    std::unique_lock l(l_basics);
    m_username = username;
    return true;
}

ThorQ::Crypto::Hashing::CalculatedHash ThorQ::Account::passwordHash() const
{
    return m_passwordHash;
}

bool ThorQ::Account::setPasswordHash(ThorQ::Crypto::Hashing::CalculatedHash hash)
{
    if (m_passwordHash == hash)
    {
        return true;
    }

    if (m_dbId <= 0)
    {
        return false;
    }

    auto dbConnection = SQLite::Connection::OpenConnection("database.db", SQLite::Connection::READWRITE);

    if (dbConnection == nullptr) {
        fmt::print(stderr, "Failed to open database\n");
        return false;
    }

    auto transaction = dbConnection->beginDeferredTransaction();

    if (!transaction.isOpen())
    {
        fmt::print(stderr, "SQL Failed to start transaction: {}\n", dbConnection->lastError());
        return false;
    }

    auto query = dbConnection->makeQuery("UPDATE OR IGNORE accounts SET password_hash = ? WHERE db_id = ? LIMIT 1"sv);

    if (!query.isValid())
    {
        fmt::print("Failed to create query: {}\n", dbConnection->lastError());
        return false;
    }

    if (!query.bindBlob(1, hash))
    {
        fmt::print("Failed to bind username: {}\n", dbConnection->lastError());
        return false;
    }

    if (!query.bindInt64(2, m_dbId))
    {
        fmt::print("Failed to bind dbID: {}\n", dbConnection->lastError());
        return false;
    }

    if (!query.step())
    {
        fmt::print("Failed to execute username query: {}\n", dbConnection->lastError());
        return false;
    }

    auto getChanges = dbConnection->makeQuery("SELECT changes();"sv);

    if (getChanges.columnCount() != 1)
    {
        fmt::print("Query didnt return any values\?\?\?\?\n");
        return false;
    }

    if (getChanges.column(1).getInt64() == 0)
    {
        fmt::print("account invalid/already used\n");
        return false;
    }

    transaction.commit();

    std::unique_lock l(l_basics);
    m_passwordHash = hash;
    return true;
}

ThorQ::Crypto::Hashing::HashingParameters ThorQ::Account::passwordHashParameters() const
{
    return m_passwordHashParameters;
}

bool ThorQ::Account::setPasswordHash(ThorQ::Crypto::Hashing::HashingParameters params)
{
    if (m_passwordHashParameters == params)
    {
        return true;
    }

    if (m_dbId <= 0)
    {
        return false;
    }

    auto dbConnection = SQLite::Connection::OpenConnection("database.db", SQLite::Connection::READWRITE);

    if (dbConnection == nullptr) {
        fmt::print(stderr, "Failed to open database\n");
        return false;
    }

    auto transaction = dbConnection->beginDeferredTransaction();

    if (!transaction.isOpen())
    {
        fmt::print(stderr, "SQL Failed to start transaction: {}\n", dbConnection->lastError());
        return false;
    }

    auto query = dbConnection->makeQuery("UPDATE OR IGNORE accounts SET password_salt = ?, password_ops_limit = ?, password_mem_limit = ?, password_algorithm = ? WHERE db_id = ? LIMIT 1"sv);

    if (!query.isValid())
    {
        fmt::print("Failed to create query: {}\n", dbConnection->lastError());
        return false;
    }

    if (!query.bindBlob(1, params.salt))
    {
        fmt::print("Failed to bind hashingparams.salt: {}\n", dbConnection->lastError());
        return false;
    }

    if (!query.bindInt64(2, params.ops_limit))
    {
        fmt::print("Failed to bind hashingparams.ops_limit: {}\n", dbConnection->lastError());
        return false;
    }

    if (!query.bindInt64(3, params.mem_limit))
    {
        fmt::print("Failed to bind hashingparams.mem_limit: {}\n", dbConnection->lastError());
        return false;
    }

    if (!query.bindInt64(4, params.algorithm))
    {
        fmt::print("Failed to bind hashingparams.algorithm: {}\n", dbConnection->lastError());
        return false;
    }

    if (!query.bindInt64(5, m_dbId))
    {
        fmt::print("Failed to bind dbID: {}\n", dbConnection->lastError());
        return false;
    }

    if (!query.step())
    {
        fmt::print("Failed to execute username query: {}\n", dbConnection->lastError());
        return false;
    }

    auto getChanges = dbConnection->makeQuery("SELECT changes()"sv);

    if (getChanges.columnCount() != 1)
    {
        fmt::print("Query didnt return any values\?\?\?\?\n");
        return false;
    }

    if (getChanges.column(1).getInt64() == 0)
    {
        fmt::print("account invalid/already used\n");
        return false;
    }

    transaction.commit();

    std::unique_lock l(l_basics);
    m_passwordHashParameters = params;
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

void ThorQ::Account::requestSession(std::shared_ptr<ThorQ::ApiServerConnection> source, std::shared_ptr<ThorQ::Account> targetAccount)
{
    std::vector<std::uint8_t> response;

    std::shared_ptr<ThorQ::Account> sourceAccount = source->account();

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

bool ThorQ::Account::addInstance(std::shared_ptr<ThorQ::ApiServerConnection> instance)
{
    std::unique_lock l(l_instances);
    return m_instances.insert(instance).second;
}

bool ThorQ::Account::removeInstance(std::shared_ptr<ThorQ::ApiServerConnection> instance)
{
    std::unique_lock l(l_instances);
    return m_instances.erase(instance) > 0;
}

bool ThorQ::Account::containsInstance(std::shared_ptr<ThorQ::ApiServerConnection> instance) const
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
