#ifndef DATABASE_H
#define DATABASE_H

#include "lsql/connection.h"

inline std::shared_ptr<SQLite::Connection> openDatabaseConneciton(SQLite::Connection::OpenMode openMode)
{
    auto dbConnection = SQLite::Connection::OpenConnection("database.db", openMode);

    if (dbConnection != nullptr) {
        dbConnection->setBusyTimeout(5000);

        using namespace std::literals;
        dbConnection->execute("PRAGMA journal_mode=WAL"sv);
    }

    return dbConnection;
}

#endif // DATABASE_H
