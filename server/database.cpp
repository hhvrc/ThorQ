#include "database.h"

#include "SQLiteCpp/SQLiteCpp.h"

bool ThorQ::DataBase::Initialize(const char *path)
{
    try
    {
        SQLite::Database db(path, SQLite::OPEN_CREATE|SQLite::OPEN_READWRITE);

        db.exec("CREATE TABLE IF NOT EXISTS system_ids("
                "db_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "system_id TEXT NOT NULL UNIQUE,"
                "banned_at DATETIME,"
                "registered_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)");   // Unique SystemID of a cmoputer
        db.exec("CREATE TABLE IF NOT EXISTS auth_tokens("
                "db_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "auth_token TEXT NOT NULL UNIQUE,"
                "system_id INTEGER NOT NULL,"
                "account_id INTEGER NOT NULL,"
                "created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)"); // Authentication Token generated at login
        db.exec("CREATE TABLE IF NOT EXISTS accounts("
                "db_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "username TEXT NOT NULL UNIQUE,"
                "password_hash TEXT NOT NULL,"
                "is_admin BOOLEAN NOT NULL DEFAULT FALSE,"
                "last_login DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
                "created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
                "updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
                "deleted_at DATETIME)");
        db.exec("CREATE TABLE IF NOT EXISTS systemid_account_map("
                "systemid_id INTEGER NOT NULL,"
                "account_id INTEGER NOT NULL,"
                "established_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)");   // Unique SystemID of a computer
        db.exec("CREATE TABLE IF NOT EXISTS account_blocks("
                "db_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "guid TEXT NOT NULL UNIQUE,"
                "blocker_id INTEGER NOT NULL,"
                "blockee_id INTEGER NOT NULL,"
                "created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)");
        db.exec("CREATE TABLE IF NOT EXISTS account_friends("
                "db_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "guid TEXT NOT NULL UNIQUE,"
                "sender_id INTEGER NOT NULL,"
                "receiver_id INTEGER NOT NULL,"
                "pending BOOLEAN NOT NULL, "
                "created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)");
        db.exec("CREATE TABLE IF NOT EXISTS userLog("
                "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
                "system_id INTEGER,"
                "account_id INTEGER,"
                "info TEXT NOT NULL)");
    }
    catch (...)
    {
        return false;
    }

    return true;
}
