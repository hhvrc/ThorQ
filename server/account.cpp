#include "account.h"

#include <hashing.h>
#include <thorq_payload_ack.h>
#include <thorq_payload_session.h>

#include "sqlite/connection.h"
#include "sqlite/transaction.h"
#include "sqlite/query.h"

#include "utils.h"
#include "instance.h"

ThorQ::Account::Account(std::int64_t dbId, THORQ_ACCOUNT_AUTHORITY authority, const char* username, const char* passwordHash)
    : m_dbId(dbId)
    , m_username(username)
    , m_passwordHash(passwordHash)
    , m_authority(authority)
    , l_sessions()
    , m_sessions()
    , l_instances()
    , m_instances()
    , l_relationships()
    , m_relationships()
{
}

ThorQ::Account* ThorQ::Account::GetAccount(const char* username)
{
    Account* account = nullptr;

    SQLite::Connection connection("database.db", SQLite::Connection::READWRITE);

    if (!connection.isOpen())
    {
        return account;
    }

    SQLite::Query query = connection.query("SELECT db_id, password_hash, authority FROM accounts WHERE username = ?");

    query.bind(1, username);

    if (!query.step())
    {
        return nullptr;
    }

    if (query.columnCount() != 3)
    {
        return nullptr;
    }


    // Get database ID
    std::int64_t dbId;
    {
        db_value = sqlite3_column_value(db_stmt, 0);
        if (sqlite3_value_type(db_value) != SQLITE_INTEGER)
        {
            sqlite3_value_free(db_value);
            sqlite3_finalize(db_stmt);
            sqlite3_close_v2(db);
            return nullptr;
        }
        dbId = sqlite3_value_int64(db_value);
        sqlite3_value_free(db_value);
    }

    // Get authority
    db_value = sqlite3_column_value(db_stmt, 2);
    if (sqlite3_value_type(db_value) != SQLITE_INTEGER)
    {
        return nullptr;
    }
    int authority = sqlite3_value_int(db_value);
    if (authority < THORQ_ACCOUNT_AUTHORITY_NONE || authority > THORQ_ACCOUNT_AUTHORITY_FOUNDER)
    {
        return nullptr;
    }

    // Get password hash
    db_value = sqlite3_column_value(db_stmt, 1);
    if (sqlite3_value_type(db_value) != SQLITE_TEXT)
    {
        return nullptr;
    }

    account = new Account(
                          dbId,
                          (THORQ_ACCOUNT_AUTHORITY)authority,
                          username,
                          (const char*)sqlite3_value_text(db_value)
                         );

    sqlite3_val
    bool convOk;
    account->m_username = username;
    account->m_dbId = sqlite3_ret get_account.value(0).toInt(&convOk);

    if (!convOk)
    {
        account->deleteLater();

        qDebug() << "Failed to get dbId";

        if (get_account.isActive()) get_account.clear();
        db.rollback();

        return nullptr;
    }

    account->m_passwordHash = get_account.value(1).toString();
    account->m_authority = static_cast<THORQ_ACCOUNT_AUTHORITY>(get_account.value(2).toInt());

    return account;
ret_err:
    const char* errStr = sqlite3_errmsg(db);
    fprintf(stderr, "SQL error: %s\n", errStr);

    if (account  != nullptr) delete account;
    if (db_value != nullptr) sqlite3_value_free(db_value);
    if (db_stmt  != nullptr) sqlite3_finalize(db_stmt);
    if (db       != nullptr) sqlite3_close_v2(db);
    return nullptr;
}

ThorQ::Account* ThorQ::Account::NewAccount(const char* username, const char* password_hash)
{
    Account* account = nullptr;

    SQLite::Connection connection("database.db", SQLite::Connection::READWRITE);

    if (!connection.isOpen())
    {
        return account;
    }

    SQLite::Transaction transaction = connection.transaction();

    if (!transaction.isOpen())
	{
        printf("Failed to start transaction: %s\n", connection.lastError());
		return nullptr;
    }

    SQLite::Query query("INSERT OR IGNORE INTO accounts(username, password_hash) VALUES (?, ?);SELECT changes();", connection);
    register_account.bindValue(0, username);
    register_account.bindValue(1, password_hash);

    if (!register_account.exec() || !register_account.isValid())
    {
        qDebug() << "Failed to execute account query:" << db.lastError();

        if (register_account.isActive()) register_account.clear();
        db.rollback();

        return nullptr;
    }

    if (!register_account.next())
    {
        qDebug() << "Query didnt return any values????";

        if (register_account.isActive()) register_account.clear();
        db.rollback();

        return nullptr;
    }

    bool convOk;
    int changes = register_account.value(0).toInt(&convOk);

    if (!convOk || changes == 0)
    {
        qDebug() << "Account invalid/already used";

        if (register_account.isActive()) register_account.clear();
        db.rollback();

        return nullptr;
    }

    if (!transaction.commit())
	{
        printf("Failed to commit account!\n");
		return nullptr;
	}

	return account;
}

const std::string& ThorQ::Account::username() const
{
	return m_username;
}

void ThorQ::Account::setUsername(const std::string& username)
{
	if (m_username != username && m_dbId != -1)
    {
        SQLite::Connection connection("database.db", SQLite::Connection::READWRITE);

        if (!connection.isOpen())
        {
            return nullptr;
        }

        Account* account = nullptr;

		if (!db.transaction())
		{
			qDebug() << "Failed to start transaction:" << db.lastError();
			db.rollback();
			return;
		}

		{
			QSqlQuery set_username("UPDATE OR IGNORE accounts SET username = ? WHERE db_id = ? LIMIT 1;SELECT changes();", db);
			set_username.bindValue(0, username);
			set_username.bindValue(1, m_dbId);

			if (!set_username.exec() || !set_username.isValid())
			{
				qDebug() << "Failed to execute regkey query:" << db.lastError();
				if (set_username.isActive()) set_username.clear();
				db.rollback();
				return;
			}

			if (!set_username.next())
			{
				qDebug() << "Query didnt return any values????";
				if (set_username.isActive()) set_username.clear();
				db.rollback();
				return;
			}

			bool convOk;
			int changes = set_username.value(0).toInt(&convOk);

			if (!convOk || changes == 0)
			{
				qDebug() << "Regkey invalid/already used";
				if (set_username.isActive()) set_username.clear();
				db.rollback();
				return;
			}
		}

		if (db.commit())
		{
			m_username = username;
			emit usernameChanged(username);
		}
	}
}

void ThorQ::Account::setPassword(const std::string& password)
{
    PasswordHasher* hasher = new PasswordHasher(password, this);

    hasher->setAutoDelete(true);

    connect(hasher, &PasswordHasher::finished, this, &Account::onPasswordHashingDone);

    QThreadPool::globalInstance()->start(hasher);
}
void ThorQ::Account::verifyPassword(const std::string& password) const
{
    PasswordVerifier* verifier = new PasswordVerifier(m_passwordHash, password, const_cast<Account*>(this));

    verifier->setAutoDelete(true);

    connect(verifier, &PasswordVerifier::finished, this, &Account::onPasswordVerificationDone);

    QThreadPool::globalInstance()->start(verifier);
}

ThorQ::Account *ThorQ::Account::master() const
{
    QReadLocker l(const_cast<QReadWriteLock*>(&l_master));
    return m_master;
}

bool ThorQ::Account::isExclusive() const
{
    return m_exclusive;
}

QSet<ThorQ::Session*> ThorQ::Account::sessions() const
{
    QReadLocker l(const_cast<QReadWriteLock*>(&l_sessions));
    return m_sessions;
}
QSet<ThorQ::Instance*> ThorQ::Account::instances() const
{
    QReadLocker l(const_cast<QReadWriteLock*>(&l_instances));
    return m_instances;
}
QSet<ThorQ::Relationship*> ThorQ::Account::relationships() const
{
    QReadLocker l(const_cast<QReadWriteLock*>(&l_relationships));
    return m_relationships;
}

void ThorQ::Account::requestSession(ThorQ::Instance* source, ThorQ::Account* target)
{
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
}
bool ThorQ::Account::requestDenyFrom(ThorQ::Account *sender)
{
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
        thorq_payload_notification_pack(message, THORQ_NOTIFICATION_USER_ACTIVITY, account()->username(), m_activityState);
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

void ThorQ::Account::setActivityState(uint8_t state)
{
    m_activityState = state;

    std::vector<std::uint8_t> message;
    thorq_payload_notification_pack(message, THORQ_NOTIFICATION_USER_ACTIVITY, account()->username(), m_activityState);
    broadcastNotification(message, true);
}

uint8_t ThorQ::Account::activityState() const
{
    return m_activityState;
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
        instance->sendMessage(message, encrypt, reliable);
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

void ThorQ::Account::onPasswordHashingDone(const std::string &hash)
{
    qDebug() << "Hash:" << hash.c_str();
}

void ThorQ::Account::onPasswordVerificationDone(bool result)
{

}
