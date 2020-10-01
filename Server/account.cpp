#include "account.h"

#include <QtSql>

#include <botan_all.h>

#include "utils.h"
#include "session.h"

ThorQ::Account::Account(QObject* parent)
	: QObject(parent)
	, m_dbId(-1)
	, m_username()
	, m_passwordHash()
//	, m_instances()
{
}

ThorQ::Account* ThorQ::Account::GetAccount(const QString& username)
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
		account->m_isAdmin = get_account.value(2).toBool();
	}
	return account;
}

ThorQ::Account* ThorQ::Account::NewAccount(const QString& username, const QString& password_hash, const QString& registrationKey)
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
		QSqlQuery use_regkey("UPDATE OR IGNORE reg_keys SET claimed_at = CURRENT_TIMESTAMP WHERE claimed_at = NULL AND reg_key = ?;SELECT changes();", db);
		use_regkey.bindValue(0, registrationKey);

		if (!use_regkey.exec() || !use_regkey.isValid())
		{
			qDebug() << "Failed to execute regkey query:" << db.lastError();

			if (use_regkey.isActive()) use_regkey.clear();
			db.rollback();

			return nullptr;
		}

		if (!use_regkey.next())
		{
			qDebug() << "Query didnt return any values????";

			if (use_regkey.isActive()) use_regkey.clear();
			db.rollback();

			return nullptr;
		}

		bool convOk;
		int changes = use_regkey.value(0).toInt(&convOk);

		if (!convOk || changes == 0)
		{
			qDebug() << "Regkey invalid/already used";

			if (use_regkey.isActive()) use_regkey.clear();
			db.rollback();

			return nullptr;
		}
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

const QString& ThorQ::Account::username() const
{
	return m_username;
}

void ThorQ::Account::setUsername(const QString& username)
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

void ThorQ::Account::setPassword(const QString& password)
{
	std::string hash;

	{
		Botan::AutoSeeded_RNG rng = Botan::AutoSeeded_RNG();
        hash = Botan::generate_bcrypt(password.toStdString(), rng);
	}

	qDebug() << "Hash:" << hash.c_str();
}

bool ThorQ::Account::verifyPassword(const QString& password)
{
	return Botan::check_bcrypt(password.toStdString(), m_passwordHash.toStdString());
}

void ThorQ::Instance::requestOn(Instance* target)
{
    std::vector<std::uint8_t> response;

    // If account already has a partner or target account is self
    if (m_partner != nullptr || target == this)
    {
        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_SESSION, THORQ_PAYLOAD_SESSION_REQUEST, THORQ_PAYLOAD_ACK_DENIED);
        sendMessage(response, true, true);
        return;
    }

    if ()
    {
        thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_DENIED, "Cannot request on self");
        sendMessage(response, true, true);
        return;
    }

    if (target->isInSession())
    {
        thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_DENIED, target->account()->username() + " is already in another session");
        sendMessage(response, true, true);
        return;
    }

    // Spam prevention
    if (m_outgoing_requests.contains(target))
    {
        thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_NO_CHANGE);
        sendMessage(response, true, true);
        return;
    }
    m_outgoing_requests.insert(target);

    target->m_incoming_requests.insert(this);

    thorq_payload_event_pack(response, THORQ_EVENT_SESSION_REQUESTED, account()->username());
    target->sendMessage(response, true, true);

    thorq_payload_ack_pack(response, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_COMMAND_ACK_RESULT_IN_PROGRESS, "Request sent");
    this->sendMessage(response, true, true);
}
bool ThorQ::Account::requestAcceptFrom(Account* sender)
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
ThorQ::Account* ThorQ::Account::partner() const
{
    return m_partner;
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
