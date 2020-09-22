#include "account.h"

#include <QtSql>

#include <botan_all.h>

#include "session.h"

inline QSqlDatabase OpenDb(bool readonly = true)
{
	QSqlDatabase db = QSqlDatabase::database();
	db.setDatabaseName("database.db");
	if (readonly) db.setConnectOptions("QSQLITE_OPEN_READONLY");
	return db;
}

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
	QSqlDatabase db = OpenDb(true);

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
	QSqlDatabase db = OpenDb(false);

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
		QSqlDatabase db = OpenDb(false);

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
