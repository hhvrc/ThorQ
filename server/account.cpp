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

#include "database.h"

#include "apiserver_connection.h"

using namespace std::literals;

std::shared_mutex g_accounts_lock;
std::unordered_map<ThorQ::Uuid, std::shared_ptr<ThorQ::Account>> g_accounts_by_uuid;
std::unordered_map<std::string, std::shared_ptr<ThorQ::Account>> g_accounts_by_username;

ThorQ::Account::Account(std::int64_t dbId, std::int64_t passwordId, ThorQ::Uuid id, const std::string& username)
    : m_uuid(id)
    , m_dbRowId(dbId)
    , m_passwordId(passwordId)
    , l_basics()
    , m_username(username)
    , m_newPasswordSalt{0}
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

std::shared_ptr<ThorQ::Account> ThorQ::Account::GetAccount(const ThorQ::Uuid& uuid)
{
    {
        std::shared_lock l(g_accounts_lock);
        auto it = g_accounts_by_uuid.find(uuid);

        if (it != g_accounts_by_uuid.end()) {
            return it->second;
        }
    }

    auto dbConnection = openDatabaseConneciton(SQLite::Connection::READONLY);
    if (dbConnection == nullptr) {
        fmt::print(stderr, "Failed to open database\n");
        return nullptr;
    }

    SQLite::Query query = dbConnection->makeQuery(
                "SELECT "
                    "accounts.account_id,"
                    "accounts.username,"
                    "accounts.password_id,"
                    "accounts.email_address,"
                    "images.uuid "
                "FROM accounts "
                    "INNER JOIN images ON images.image_id = accounts.image_id "
                "WHERE accounts.uuid = ?"sv);

    if (!query.bindText(1, uuid.toString()) ||
        !query.step()) {
        fmt::print(stderr, "SQL Failed to execute get account query: {}\n", dbConnection->lastError());
        return nullptr;
    }

    if (query.columnCount() == 0) {
        fmt::print("SQL account query returned no results\n");
        return nullptr;
    }

    auto dbId = query.column(0).getInt64();
    auto username = query.column(1).getDataText();
    auto passwordId = query.column(2).getInt64();

    auto account = std::shared_ptr<ThorQ::Account>(new ThorQ::Account(dbId, passwordId, uuid, username));

    if (!ThorQ::Uuid::TryParse(query.column(4).getDataText(), account->m_imageId)) {
        fmt::print(stderr, "Failed to parse account imageid: {}\n", query.column(6).getDataText());
        return nullptr;
    }

    auto emailAddrCol = query.column(3);
    if (emailAddrCol.type() == SQLite::Type::Text) {
        account->m_emailAddress = emailAddrCol.getDataText();
    }

    std::scoped_lock l(g_accounts_lock);
    g_accounts_by_uuid.insert({uuid, account});
    g_accounts_by_username.insert({username, account});
    return account;
}

std::shared_ptr<ThorQ::Account> ThorQ::Account::GetAccount(const std::string& username)
{
    {
        std::shared_lock l(g_accounts_lock);
        auto it = g_accounts_by_username.find(username);

        if (it != g_accounts_by_username.end()) {
            return it->second;
        }
    }

    auto dbConnection = openDatabaseConneciton(SQLite::Connection::READONLY);
    if (dbConnection == nullptr) {
        fmt::print(stderr, "Failed to open database\n");
        return nullptr;
    }

    SQLite::Query query = dbConnection->makeQuery(
                "SELECT "
                    "accounts.account_id,"
                    "accounts.uuid,"
                    "accounts.password_id,"
                    "accounts.email_address,"
                    "images.uuid "
                "FROM accounts "
                    "INNER JOIN "
                        "images "
                    "ON "
                        "images.image_id = accounts.image_id "
                "WHERE "
                    "accounts.username = ?"sv);

    if (!query.bindText(1, username) ||
        !query.step()) {
        fmt::print(stderr, "SQL Failed to execute get account query: {}\n", dbConnection->lastError());
        return nullptr;
    }

    if (query.columnCount() == 0) {
        fmt::print("SQL account query returned no results\n");
        return nullptr;
    }

    auto dbId = query.column(0).getInt64();

    auto uuidCol = query.column(1);
    if (uuidCol.type() != SQLite::Type::Text) {
        fmt::print(stderr, "SQL account query returned invalid uuid type ({})\n", uuidCol.type());
        return nullptr;
    }
    ThorQ::Uuid uuid;
    if (!ThorQ::Uuid::TryParse(uuidCol.getDataText(), uuid)) {
        fmt::print("Uuid from sql is unparsable ({})\n", uuidCol.getDataText());
        return nullptr;
    }

    auto passwordId = query.column(2).getInt64();

    auto account = std::shared_ptr<ThorQ::Account>(new ThorQ::Account(dbId, passwordId, uuid, username));

    if (!ThorQ::Uuid::TryParse(query.column(4).getDataText(), account->m_imageId)) {
        fmt::print(stderr, "Failed to parse account imageid\n");
        return nullptr;
    }

    auto emailAddrCol = query.column(3);
    if (emailAddrCol.type() == SQLite::Type::Text) {
        account->m_emailAddress = emailAddrCol.getDataText();
    }

    std::scoped_lock l(g_accounts_lock);
    g_accounts_by_uuid.insert({uuid, account});
    g_accounts_by_username.insert({username, account});
    return account;
}

std::shared_ptr<ThorQ::Account> ThorQ::Account::NewAccount(const std::string& username)
{
    {
        std::shared_lock l(g_accounts_lock);
        auto it = g_accounts_by_username.find(username);

        if (it != g_accounts_by_username.end())
        {
            fmt::print("Account already exists: {}\n", username);
            return nullptr;
        }
    }

    auto dbConnection = openDatabaseConneciton(SQLite::Connection::READWRITE);
    if (dbConnection == nullptr) {
        fmt::print(stderr, "Failed to open database\n");
        return nullptr;
    }

    auto transaction = dbConnection->beginDeferredTransaction();

    if (!transaction.isOpen()) {
        fmt::print(stderr, "SQL Failed to start transaction: {}\n", dbConnection->lastError());
		return nullptr;
    }

    SQLite::Query passwordQuery = dbConnection->makeQuery("INSERT INTO passwords(salt, ops_limit, mem_limit, algorithm) VALUES (?, ?, ?, ?)"sv);

    ThorQ::Crypto::Hashing::Salt hashingSalt;
    ThorQ::Crypto::Hashing::Parameters hashingParams;
    hashingParams.setPerformance(ThorQ::Crypto::Hashing::Parameters::Performance::Sensitive);
    randombytes_buf(hashingSalt.data(), ThorQ::Crypto::Hashing::SaltLength);

    if (!passwordQuery.bindBlob(1, hashingSalt)              ||
        !passwordQuery.bindInt64(2, hashingParams.ops_limit) ||
        !passwordQuery.bindInt64(3, hashingParams.mem_limit) ||
        !passwordQuery.bindInt32(4, hashingParams.algorithm) ||
        !passwordQuery.step()) {
        fmt::print(stderr, "SQL Failed to execute set password hash query: {}\n", dbConnection->lastError());
        return nullptr;
    }
    std::int64_t passwordId = dbConnection->lastInsertedRowId();

    SQLite::Query accountQuery = dbConnection->makeQuery("INSERT INTO accounts(uuid, username, password_id, image_id) VALUES (?, ?, ?, 1)"sv);

    ThorQ::Uuid uuid = ThorQ::Uuid::NewUuid();
    if (!accountQuery.bindText(1, uuid.toString()) ||
        !accountQuery.bindText(2, username)        ||
        !accountQuery.bindInt64(3, passwordId)     ||
        !accountQuery.step()) {
        fmt::print(stderr, "SQL Failed to execute account query: {}\n", dbConnection->lastError());
        return nullptr;
    }

    std::int64_t dbId = dbConnection->lastInsertedRowId();

    if (dbId <= 0)
    {
        fmt::print(stderr, "username [{}] not available\n", username);
        return nullptr;
    }

    if (!transaction.commit())
	{
        fmt::print(stderr, "SQL Failed to commit account: {}\n", dbConnection->lastError());
		return nullptr;
	}

    fmt::print("[ACCOUNT] Created: {}\n", username);
    auto account = std::shared_ptr<ThorQ::Account>(new ThorQ::Account(dbId, passwordId, uuid, username));

    std::scoped_lock l(g_accounts_lock);
    g_accounts_by_uuid.insert({uuid, account});
    g_accounts_by_username.insert({username, account});
    return account;
}

std::string ThorQ::Account::username() const
{
	return m_username;
}

bool ThorQ::Account::setUsername(const std::string& newUsername)
{
    if (m_username == newUsername)
    {
        return true;
    }

    auto dbConnection = openDatabaseConneciton(SQLite::Connection::READWRITE);
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

    SQLite::Query query = dbConnection->makeQuery(
                "UPDATE "
                    "accounts "
                "SET "
                    "username = ?,"
                    "updated_at = CURRENT_TIMESTAMP "
                "WHERE "
                    "account_id = ?"sv);

    if (!query.isValid()               ||
        !query.bindText(1, newUsername)   ||
        !query.bindInt64(2, m_dbRowId) ||
        !query.step()                  ||
         dbConnection->changes() < 1
         ) {
        fmt::print("account name taken\n");
        return false;
    }

    if (!transaction.commit()) {
        return false;
    }

    std::unique_lock sl_basics(l_basics);
    auto oldUsername = m_username;                                  // Get old username
    m_username = newUsername;                                       // Set new username

    std::shared_lock sl_accounts(g_accounts_lock);
    auto node = g_accounts_by_username.extract(oldUsername);        // Find account node

    if (node.empty()) {
        return false;
    }

    node.key() = newUsername;                                       // Set new key
    return g_accounts_by_username.insert(std::move(node)).inserted; // Insert modified node
}

bool ThorQ::Account::isClaimed() const
{
    auto dbConnection = openDatabaseConneciton(SQLite::Connection::READONLY);
    if (dbConnection == nullptr) {
        fmt::print(stderr, "Failed to open database\n");
        return false;
    }

    SQLite::Query query = dbConnection->makeQuery(
                "SELECT "
                    "account_id "
                "FROM "
                    "accounts "
                "WHERE "
                    "account_id = ? AND email_address IS NULL "
                "LIMIT 1"sv);

    if (!query.bindInt64(1, m_dbRowId) ||
        !query.step()
         ) {
        fmt::print("SQL check account claimed failed: {}\n", dbConnection->lastError());
        return false;
    }

    return query.columnCount() == 1;
}

bool ThorQ::Account::tryClaim(const std::string& emailAddress, const ThorQ::Crypto::Hashing::Hash& passwordHash, const ThorQ::Crypto::Hashing::Salt& passwordSalt, const ThorQ::Crypto::Hashing::Parameters& passwordHashingParameters)
{
    if (passwordHashingParameters.ops_limit <= 0 ||
        passwordHashingParameters.mem_limit <= 0 ||
        passwordHashingParameters.algorithm <= 0
        ) {
        fmt::print(stderr, "Invalid hashing parameters!\n");
        return false;
    }

    auto dbConnection = openDatabaseConneciton(SQLite::Connection::READWRITE);
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

    SQLite::Query updateAccountQuery = dbConnection->makeQuery(
                "UPDATE "
                    "accounts "
                "SET "
                    "email_address = ?,"
                    "updated_at = CURRENT_TIMESTAMP "
                "WHERE "
                    "account_id = ? AND email_address IS NULL"sv);

    if (!updateAccountQuery.bindText(1, emailAddress) ||
        !updateAccountQuery.bindInt64(2, m_dbRowId)   ||
        !updateAccountQuery.step()                    ||
         dbConnection->changes() < 1
         ) {
        fmt::print("SQL claim account (update account) failed: {}\n", dbConnection->lastError());
        return false;
    }

    if (dbConnection->changes() < 1) {
        fmt::print("Account already claimed\n");
        return false;
    }

    std::shared_lock l(l_basics);
    if (passwordSalt != m_newPasswordSalt) {
        fmt::print(stderr, "Invalid hashing salt\n");
        return false;
    }

    SQLite::Query updatePasswordQuery = dbConnection->makeQuery(
                "UPDATE "
                    "passwords "
                "SET "
                    "salt = ?,"
                    "hash = ?,"
                    "ops_limit = ?,"
                    "mem_limit = ?,"
                    "algorithm = ? "
                "WHERE "
                    "password_id = ? AND hash IS NULL"sv);

    if (!updatePasswordQuery.bindBlob(1, passwordSalt)                         ||
        !updatePasswordQuery.bindBlob(2, passwordHash)                         ||
        !updatePasswordQuery.bindInt64(3, passwordHashingParameters.ops_limit) ||
        !updatePasswordQuery.bindInt64(4, passwordHashingParameters.mem_limit) ||
        !updatePasswordQuery.bindInt32(5, passwordHashingParameters.algorithm) ||
        !updatePasswordQuery.bindInt64(6, m_passwordId)                        ||
        !updatePasswordQuery.step()                                            ||
         dbConnection->changes() < 1
         ) {
        fmt::print("SQL claim account (update password) failed: {}\n", dbConnection->lastError());
        return false;
    }

    if (!transaction.commit()) {
        return false;
    }

    m_emailAddress = emailAddress;

    return true;
}

ThorQ::Crypto::Hashing::Salt ThorQ::Account::newPasswordSalt()
{
    randombytes_buf(m_newPasswordSalt.data(), ThorQ::Crypto::Hashing::SaltLength);
    return m_newPasswordSalt;
}
ThorQ::Crypto::Hashing::Salt ThorQ::Account::currentPasswordSalt() const
{
    auto dbConnection = openDatabaseConneciton(SQLite::Connection::READONLY);
    if (dbConnection == nullptr) {
        fmt::print(stderr, "Failed to open database\n");
        return {};
    }

    SQLite::Query selectSaltQuery = dbConnection->makeQuery(
                "SELECT "
                    "salt "
                "FROM "
                    "passwords "
                "WHERE "
                    "password_id = ?"sv);

    if (!selectSaltQuery.bindInt64(1, m_passwordId) ||
        !selectSaltQuery.step()                     ||
         selectSaltQuery.columnCount() < 1
         ) {
        fmt::print(stderr, "SQL get password salt failed: {}\n", dbConnection->lastError());
        return {};
    }

    auto saltColumn = selectSaltQuery.column(0);
    if (saltColumn.type() != SQLite::Type::Blob) {
        fmt::print(stderr, "SQL get password salt returned value invlaid: {}\n", dbConnection->lastError());
        return {};
    }

    ThorQ::Crypto::Hashing::Salt salt;
    memcpy(salt.data(), saltColumn.getDataBlob(), ThorQ::Crypto::Hashing::SaltLength);
    return salt;
}
ThorQ::Crypto::Hashing::Parameters ThorQ::Account::passwordHashParameters() const
{
    auto dbConnection = openDatabaseConneciton(SQLite::Connection::READONLY);
    if (dbConnection == nullptr) {
        fmt::print(stderr, "Failed to open database\n");
        return {};
    }

    SQLite::Query selectParamsQuery = dbConnection->makeQuery(
                "SELECT "
                    "ops_limit,"
                    "mem_limit,"
                    "algorithm "
                "FROM "
                    "passwords "
                "WHERE "
                    "password_id = ?"sv);

    if (!selectParamsQuery.bindInt64(1, m_passwordId) ||
        !selectParamsQuery.step()                     ||
         selectParamsQuery.columnCount() < 1
         ) {
        fmt::print(stderr, "SQL get password salt failed: {}\n", dbConnection->lastError());
        return {};
    }

    ThorQ::Crypto::Hashing::Parameters parameters;
    parameters.ops_limit = selectParamsQuery.column(0).getInt64();
    parameters.mem_limit = selectParamsQuery.column(1).getInt64();
    parameters.algorithm = selectParamsQuery.column(2).getInt32();

    return parameters;
}

bool ThorQ::Account::checkPasswordHash(ThorQ::Crypto::Hashing::HashRef hash)
{
    auto dbConnection = openDatabaseConneciton(SQLite::Connection::READONLY);
    if (dbConnection == nullptr) {
        fmt::print(stderr, "Failed to open database\n");
        return false;
    }

    auto checkPasswordQuery = dbConnection->makeQuery(
                "SELECT "
                    "password_id "
                "FROM "
                    "passwords "
                "WHERE "
                    "password_id = ? AND hash = ?"sv);

    if (!checkPasswordQuery.bindInt64(1, m_passwordId) ||
        !checkPasswordQuery.bindBlob(2, hash)          ||
        !checkPasswordQuery.step()                     ||
         checkPasswordQuery.columnCount() != 1
         ) {
        fmt::print(stderr, "Failed to verify password match\n");
        return false;
    }

    return true;
}

bool ThorQ::Account::tryUpdatePassword(ThorQ::Crypto::Hashing::HashRef oldPwHash, ThorQ::Crypto::Hashing::HashRef newPwHash, ThorQ::Crypto::Hashing::Parameters newHashingParams, ThorQ::Crypto::Hashing::SaltRef expectedNewSalt)
{
    ThorQ::Crypto::Hashing::Salt newSalt;
    {
        std::shared_lock l(l_basics);
        newSalt = m_newPasswordSalt;
    }

    if (memcmp(newSalt.data(), expectedNewSalt.data(), ThorQ::Crypto::Hashing::SaltLength) != 0) {
        // TODO password cahnged
        return false;
    }

    auto dbConnection = openDatabaseConneciton(SQLite::Connection::READWRITE);
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

    auto updatePasswordQuery = dbConnection->makeQuery(
                "UPDATE "
                    "passwords "
                "SET "
                    "hash = ?,"
                    "salt = ?,"
                    "ops_limit = ?,"
                    "mem_limit = ?,"
                    "algorithm = ? "
                "WHERE"
                    "password_id = ? AND hash = ?"sv);

    if (!updatePasswordQuery.isValid())
    {
        fmt::print("Failed to create query: {}\n", dbConnection->lastError());
        return false;
    }

    if (!updatePasswordQuery.bindBlob(1, newPwHash)                   ||
        !updatePasswordQuery.bindBlob(2, newSalt)                     ||
        !updatePasswordQuery.bindInt64(3, newHashingParams.ops_limit) ||
        !updatePasswordQuery.bindInt64(4, newHashingParams.mem_limit) ||
        !updatePasswordQuery.bindInt32(5, newHashingParams.algorithm) ||
        !updatePasswordQuery.bindInt64(6, m_passwordId)               ||
        !updatePasswordQuery.bindBlob(7, oldPwHash)                   ||
        !updatePasswordQuery.step())
    {
        fmt::print("Failed to execute updatePassword query: {}\n", dbConnection->lastError());
        return false;
    }

    if (dbConnection->changes() < 1) {
        fmt::print("password invalid, will not update!\n");
        return false;
    }

    transaction.commit();
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
