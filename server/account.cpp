#include "account.h"

#include <QtSql>

#include <hasher.h>
#include <thorq_payload_ack.h>
#include <thorq_payload_session.h>

#include "utils.h"
#include "instance.h"

ThorQ::Account::Account()
    : m_dbId(-1)
	, m_username()
	, m_passwordHash()
    , l_sessions(QReadWriteLock::Recursive)
    , m_sessions()
    , l_instances(QReadWriteLock::Recursive)
    , m_instances()
    , l_relationships(QReadWriteLock::Recursive)
    , m_relationships()
{
}

ThorQ::Account* ThorQ::Account::GetAccount(const std::string& username)
{
    QSqlDatabase db = GetDB(true);

	Account* account = nullptr;

	if (!db.open())
	{
		qDebug() << "Failed to open database:" << db.lastError();
		return nullptr;
	}

	{
		QSqlQuery get_account("SELECT db_id, password_hash, is_admin FROM accounts WHERE username = ?;", db);
		get_account.bindValue(0, username);

		if (!get_account.exec() || !get_account.isValid())
		{
			qDebug() << "Failed to execute account query:" << db.lastError();

			if (get_account.isActive()) get_account.clear();
			db.rollback();

			return nullptr;
		}

		if (!get_account.next())
		{
			qDebug() << "Query didnt return any values????";

			if (get_account.isActive()) get_account.clear();
			db.rollback();

			return nullptr;
		}

		account = new Account();

		bool convOk;
		account->m_username = username;
		account->m_dbId = get_account.value(0).toInt(&convOk);

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
	}
	return account;
}

ThorQ::Account* ThorQ::Account::NewAccount(const std::string& username, const std::string& password_hash)
{
    QSqlDatabase db = GetDB(false);

	Account* account = nullptr;

	if (!db.open())
	{
		qDebug() << "Failed to open database:" << db.lastError();
		return nullptr;
	}

	if (!db.transaction())
	{
		qDebug() << "Failed to start transaction:" << db.lastError();
		db.rollback();
		return nullptr;
    }

	{
		QSqlQuery register_account("INSERT OR IGNORE INTO accounts(username, password_hash) VALUES (?, ?);SELECT changes();", db);
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
	}

	if (!db.commit())
	{
		qDebug() << "Failed to commit account!";

		db.rollback();

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
        QSqlDatabase db = GetDB(false);

		if (!db.open())
		{
			qDebug() << "Failed to open database:" << db.lastError();
			return;
		}

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
