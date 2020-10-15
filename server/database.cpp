#include "database.h"

#include <cstdio>

#include "sqlite3/sqlite3.h"

const char* create_system_ids    = "CREATE TABLE IF NOT EXISTS system_ids(db_id INTEGER PRIMARY KEY AUTOINCREMENT, system_id TEXT NOT NULL UNIQUE, banned_at DATETIME, registered_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)"; // Unique SystemID of a cmoputer
const char* create_auth_tokens   = "CREATE TABLE IF NOT EXISTS auth_tokens(db_id INTEGER PRIMARY KEY AUTOINCREMENT, auth_token TEXT NOT NULL UNIQUE, system_id INTEGER NOT NULL, account_id INTEGER NOT NULL, created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)"; // Authentication Token generated at login
const char* create_accounts      = "CREATE TABLE IF NOT EXISTS accounts(db_id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT NOT NULL UNIQUE, password_hash TEXT NOT NULL, authority INTEGER NOT NULL DEFAULT 0, last_login DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, deleted_at DATETIME)";
const char* create_systemid_account_map = "CREATE TABLE IF NOT EXISTS systemid_account_map(systemid_id INTEGER NOT NULL, account_id INTEGER NOT NULL, established_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)";
const char* create_account_blocks = "CREATE TABLE IF NOT EXISTS account_blocks(db_id INTEGER PRIMARY KEY AUTOINCREMENT, guid TEXT NOT NULL UNIQUE, blocker_id INTEGER NOT NULL, blockee_id INTEGER NOT NULL, created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)";
const char* create_account_friends = "CREATE TABLE IF NOT EXISTS account_friends(db_id INTEGER PRIMARY KEY AUTOINCREMENT, guid TEXT NOT NULL UNIQUE, sender_id INTEGER NOT NULL, receiver_id INTEGER NOT NULL, pending BOOLEAN NOT NULL,  created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)";
const char* create_user_log = "CREATE TABLE IF NOT EXISTS userLog(timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, system_id INTEGER, account_id INTEGER, info TEXT NOT NULL)";


ThorQ::SQLite::Connection::Connection(const char* path)
    : m_path(path)
    , m_db(nullptr)
{
}

ThorQ::SQLite::Connection::~Connection()
{
    if (m_db != nullptr)
    {
        sqlite3_close_v2(m_db);
    }
}

bool ThorQ::SQLite::Connection::open(OpenMode openMode)
{
    int flags = SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE;

    switch (openMode) {
    case OpenMode::READONLY:
        flags |= SQLITE_OPEN_READONLY;
        break;
    case OpenMode::CREATE:
        flags |= SQLITE_OPEN_CREATE;
    case OpenMode::READWRITE:
        flags |= SQLITE_OPEN_READWRITE;
        break;
    }


    if (!sqlite3_open_v2(m_path, &m_db, flags, ""))
    {
        sqlite3_close_v2(m_db);
        m_db = nullptr;

        return false;
    }

    return true;
}


bool execSimple(sqlite3* db, const char* query, std::size_t queryLen)
{
    const char* tail;
    sqlite3_stmt* pStmt;

    int errc = sqlite3_prepare_v3(db, query, queryLen, 0, &pStmt, &tail);

    if (errc == SQLITE_OK)
    {
        errc &= sqlite3_step(pStmt);
    }

    errc &= sqlite3_finalize(pStmt);

    return errc == SQLITE_OK;
}

bool ThorQ::SQLite::Initialize(const char* path) noexcept
{
    sqlite3* db;

    try
    {
        sqlite3_enable_shared_cache(true);

        if (sqlite3_open_v2(path, &db, SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE|SQLITE_OPEN_NOMUTEX, "") != SQLITE_OK)
        {
            sqlite3_close_v2(db);
            return false;
        }

        if (
            execSimple(db, create_system_ids,           sizeof(create_system_ids))           &&
            execSimple(db, create_auth_tokens,          sizeof(create_auth_tokens))          &&
            execSimple(db, create_accounts,             sizeof(create_accounts))             &&
            execSimple(db, create_systemid_account_map, sizeof(create_systemid_account_map)) &&
            execSimple(db, create_account_blocks,       sizeof(create_account_blocks))       &&
            execSimple(db, create_account_friends,      sizeof(create_account_friends))      &&
            execSimple(db, create_user_log,             sizeof(create_user_log))
           )
        {
            sqlite3_close_v2(db);
            return false;
        }

        sqlite3_close_v2(db);
    }
    catch (...)
    {
        *err = "unknown exception occured";
        return false;
    }

    return true;
}

bool ThorQ::Connection::ExecuteNonQuery(const char *statement)
{

}
