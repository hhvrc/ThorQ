#include "account.h"

#include <set>
#include <memory>
#include <atomic>
#include <thread>
#include <future>
#include <tbb/concurrent_unordered_map.h>

#include <hashing.h>
#include <fmt/core.h>
#include <thorq_payload_ack.h>
#include <thorq_payload_session.h>
#include <thorq_payload_relationship.h>

#include "sqlite/connection.h"
#include "sqlite/transaction.h"
#include "sqlite/column.h"
#include "sqlite/query.h"

#include "utils.h"
#include "instance.h"

tbb::concurrent_unordered_map<std::string, std::shared_ptr<ThorQ::Account>> g_accounts;

ThorQ::Account::Account(std::int64_t dbId, THORQ_ACCOUNT_AUTHORITY authority, const std::string& username, const std::string& passwordHash)
    : m_dbId(dbId)
    , m_username(username)
    , m_passwordHash(passwordHash)
    , m_authority(authority)
    , m_sessions()
    , m_instances()
    , m_relationships()
{
}

std::shared_ptr<ThorQ::Account> ThorQ::Account::GetAccount(const std::string& username)
{
    {
        std::shared_lock l(g_accounts_lock);
        auto it = std::find_if(g_accounts.begin(), g_accounts.end(), [username](const Account* account) -> bool { return account->username() == username; });

        if (it != g_accounts.end())
        {
            return *it;
        }
    }

    SQLite::Connection connection("database.db", SQLite::Connection::READWRITE);

    if (!connection.isOpen())
    {
        fmt::print(stderr, "SQL error: {}\n", connection.lastError());
        return nullptr;
    }

    SQLite::Query query = connection.query("SELECT db_id, password_hash, authority FROM accounts WHERE username = ?");

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
    SQLite::Column col = query.column(0);
    if (query.getType(0) != SQLite::Type::Integer ||
        query.getType(1) != SQLite::Type::Text    ||
        query.getType(2) != SQLite::Type::Integer)
    {
        return nullptr;
    }

    // Get authority
    int authority = col.getInt();
    if (authority < THORQ_ACCOUNT_AUTHORITY_NONE || authority > THORQ_ACCOUNT_AUTHORITY_FOUNDER)
    {
        return nullptr;
    }

    return new Account(query.column(0).getInt64(), (THORQ_ACCOUNT_AUTHORITY)authority, username, query.column(1).getText());
}

ThorQ::Account* ThorQ::Account::NewAccount(const std::string& username, const std::string& passwordHash)
{
    {
        std::shared_lock l(g_accounts_lock);
        auto it = std::find_if(g_accounts.begin(), g_accounts.end(), [username](const Account* account) -> bool { return account->username() == username; });

        if (it != g_accounts.end())
        {
            return *it;
        }
    }

    Account* account = nullptr;

    SQLite::Connection connection("database.db", SQLite::Connection::READWRITE);

    if (!connection.isOpen())
    {
        return account;
    }

    SQLite::Transaction transaction = connection.transaction();

    if (!transaction.isOpen())
	{
        fmt::print(stderr, "SQL Failed to start transaction: {}\n", connection.lastError());
		return nullptr;
    }

    SQLite::Query query = connection.query("INSERT OR IGNORE INTO accounts(username, password_hash) VALUES (?, ?);SELECT changes();");

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

    if (query.columnCount() != 1)
    {
        fmt::print(stderr, "SQL Query didnt return any values\?\?\?\? lastError: {}\n", connection.lastError());
        return nullptr;
    }

    if (query.column(1).getInt() == 0)
    {
        fmt::print(stderr, "account [{}] invalid/already used\n", username, connection.lastError());
        return nullptr;
    }

    if (!transaction.commit())
	{
        fmt::print(stderr, "SQL Failed to commit account: {}\n", connection.lastError());
		return nullptr;
	}

    return account;
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

    SQLite::Connection connection("database.db", SQLite::Connection::READWRITE);

    if (!connection.isOpen())
    {
        return false;
    }

    SQLite::Query query = connection.query("UPDATE OR IGNORE accounts SET username = ? WHERE db_id = ? LIMIT 1;SELECT changes();");

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

    SQLite::Connection connection("database.db", SQLite::Connection::READWRITE);

    if (!connection.isOpen())
    {
        return false;
    }

    SQLite::Query query = connection.query("UPDATE OR IGNORE accounts SET password_hash = ? WHERE db_id = ? LIMIT 1;SELECT changes();");

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

ThorQ::Account *ThorQ::Account::master() const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_master));
    return m_master;
}

bool ThorQ::Account::isExclusive() const
{
    return m_exclusive;
}

std::unordered_set<ThorQ::Session*> ThorQ::Account::sessions() const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_sessions));
    return m_sessions;
}
std::unordered_set<ThorQ::Instance*> ThorQ::Account::instances() const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_instances));
    return m_instances;
}
std::unordered_set<ThorQ::Relationship*> ThorQ::Account::relationships() const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_relationships));
    return m_relationships;
}

void ThorQ::Account::requestSession(std::shared_ptr<ThorQ::Instance> source, std::shared_ptr<ThorQ::Account> target)
{
    /*
    std::vector<std::uint8_t> response;

    // If account already has a partner or target account is self
    if (source->account() == target)
    {
        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_SESSION, THORQ_PAYLOAD_SESSION_REQUEST, THORQ_PAYLOAD_ACK_DENIED);
        source->sendMessage(response, THORQ_CHANNEL_MAIN, true, true);
        return;
    }

    if ()
    {
        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_SESSION, THORQ_PAYLOAD_SESSION_REQUEST, THORQ_COMMAND_ACK_DENIED);
        source->sendMessage(response, THORQ_CHANNEL_MAIN, true, true);
        return;
    }

    if (target->isInSession())
    {
        thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_DENIED, target->account()->username() + " is already in another session");
        source->sendMessage(response, THORQ_CHANNEL_MAIN, true, true);
        return;
    }

    // Spam prevention
    if (m_outgoing_requests.contains(target))
    {
        thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_NO_CHANGE);
        source->sendMessage(response, THORQ_CHANNEL_MAIN, true, true);
        return;
    }
    m_outgoing_requests.insert(target);

    target->m_incoming_requests.insert(this);

    thorq_payload_event_pack(response, THORQ_EVENT_SESSION_REQUESTED, account()->username());
    target->sendMessage(response, true, true);

    thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_IN_PROGRESS, "Request sent");
    this->sendMessage(response, true, true);
}
bool ThorQ::Account::requestAcceptFrom(ThorQ::Account* sender)
{
    std::vector<std::uint8_t> response;

    if (sender == this)
    {
        thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_ACCEPT, THORQ_COMMAND_ACK_RESULT_DENIED, "Cannot start session with self");
        sendMessage(response, true, true);
        return false;
    }

    if (!m_incoming_requests.remove(sender))
    {
        thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_ACCEPT, THORQ_COMMAND_ACK_RESULT_DENIED, " Request from " + sender->account()->username() + " is invalid / never got sent");
        sendMessage(response, true, true);
        return false;
    }
    sender->m_outgoing_requests.remove(this);

    if (m_partner != nullptr)
    {
        thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_ACCEPT, THORQ_COMMAND_ACK_RESULT_DENIED, "You are already in another session");
        sendMessage(response, true, true);
        return false;
    }

    if (!sender->isInSession())
    {
        thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_ACCEPT, THORQ_COMMAND_ACK_RESULT_DENIED, sender->account()->username() + " is already in another session");
        sendMessage(response, true, true);
        return false;
    }

    m_partner = sender;

    setSessionState(THORQ_STATE_SESSION_ACTIVE);

    thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_OK, "Request accepted");
    m_partner->sendMessage(response, true, true);
    thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_ACCEPT, THORQ_COMMAND_ACK_RESULT_OK, "Session started");
    this->sendMessage(response, true, true);

    return true;
    */
}
bool ThorQ::Account::requestDenyFrom(ThorQ::Account *sender)
{
    /*
    std::vector<std::uint8_t> response;

    if (sender == this)
    {
        thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_DENY, THORQ_COMMAND_ACK_RESULT_DENIED, "Cannot deny session with self");
        sendMessage(response, true, true);
        return false;
    }

    if (!m_incoming_requests.remove(sender))
    {
        thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_DENY, THORQ_COMMAND_ACK_RESULT_DENIED, " Request from " + sender->account()->username() + " is invalid / never got sent");
        sendMessage(response, true, true);
        return false;
    }
    sender->m_outgoing_requests.remove(this);

    thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_DENIED, "Request denied");
    sender->sendMessage(response, true, true);
    thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_DENY, THORQ_COMMAND_ACK_RESULT_OK);
    this->sendMessage(response, true, true);

    return true;
    */
}

void ThorQ::Account::setIsInSteamVR(bool value)
{
    if (isInSteamVR() != value)
    {
        if (value)
            m_activityState |= THORQ_USER_ACTIVITY_FLAG_COLLAR_PRESENT;
        else
            m_activityState &= ~THORQ_USER_ACTIVITY_FLAG_COLLAR_PRESENT;

        std::vector<std::uint8_t> message;
        thorq_payload_ _pack(message, THORQ_PAYLOAD_FRIEND_EVENT_STATUS, username(), m_activityState);
        broadcastNotification(message, true);
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

        std::vector<std::uint8_t> message;
        thorq_payload_notification_pack(message, THORQ_NOTIFICATION_USER_ACTIVITY, account()->username(), m_activityState);
        broadcastNotification(message, true);
    }
}

bool ThorQ::Account::isInSession() const
{
    return (m_activityState & THORQ_USER_ACTIVITY_FLAG_IN_SESSION) != 0;
}

bool ThorQ::Account::isInSteamVR() const
{
    return (m_activityState & THORQ_USER_ACTIVITY_FLAG_OPENVR_RUNNING) != 0;
}

bool ThorQ::Account::hasCollar() const
{
    return (m_activityState & THORQ_USER_ACTIVITY_FLAG_COLLAR_PRESENT) != 0;
}

void ThorQ::Account::setStatus(std::uint16_t flags)
{

}

void ThorQ::Account::sendMessage(const std::vector<std::uint8_t>& message, bool encrypt, bool reliable)
{
    auto recepients = instances();

    for (Instance* instance : recepients)
    {
        instance->sendPayload(message, encrypt, reliable);
    }
}

void ThorQ::Account::sendMessageToFriends(const std::vector<uint8_t> &message, bool encrypt, bool reliable)
{
    auto recepients = relationships();

    for (Relationship* relationship : recepients)
    {
        relationship;
    }
}
