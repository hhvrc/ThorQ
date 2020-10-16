#include <iostream>
#include <exception>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <atomic>
#include <algorithm>
#include <cstdio>

#if __linux__
#include <unistd.h>
#endif

#include "server.h"
#include "messagedispatcher.h"

#include "cxxopts.hpp"

#define SINGLETON_BASE
#include "singletons.h"
#include "account.h"
#include "instance.h"
#include "database.h"
#include "statistics.h"
#include "eventhandlers.h"
#include <thorq_message.h>

#define PARSE_PORT false
#define SERVER_MAX_CONNECTIONS 1024

std::atomic_bool runServer = true;

const char* create_system_ids    = "CREATE TABLE IF NOT EXISTS system_ids(db_id INTEGER PRIMARY KEY AUTOINCREMENT, system_id TEXT NOT NULL UNIQUE, banned_at DATETIME, registered_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)"; // Unique SystemID of a cmoputer
const char* create_auth_tokens   = "CREATE TABLE IF NOT EXISTS auth_tokens(db_id INTEGER PRIMARY KEY AUTOINCREMENT, auth_token TEXT NOT NULL UNIQUE, system_id INTEGER NOT NULL, account_id INTEGER NOT NULL, created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)"; // Authentication Token generated at login
const char* create_accounts      = "CREATE TABLE IF NOT EXISTS accounts(db_id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT NOT NULL UNIQUE, password_hash TEXT NOT NULL, authority INTEGER NOT NULL DEFAULT 0, last_login DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, deleted_at DATETIME)";
const char* create_relationships = "CREATE TABLE IF NOT EXISTS relationships(db_id INTEGER PRIMARY KEY AUTOINCREMENT, uuid TEXT NOT NULL UNIQUE, source INTEGER NOT NULL, target INTEGER NOT NULL, created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)";
const char* create_systemid_account_map = "CREATE TABLE IF NOT EXISTS systemid_account_map(systemid_id INTEGER NOT NULL, account_id INTEGER NOT NULL, established_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)";
const char* create_user_log = "CREATE TABLE IF NOT EXISTS userLog(timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, system_id INTEGER, account_id INTEGER, info TEXT NOT NULL)";


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
        return false;
    }

    return true;
}
void exit_handler(int s)
{
    char buf[64];
    strerror_s(buf, 64, s);

    printf("Caught signal %s\n", buf);

    runServer.store(false);
}

void exitCleanup()
{
    g_server.cleanup();
}

int main(int argc, char** argv)
{
    printf("ThorQ Server %s\n", THORQ_VERSION_SERVER.toString().)

    cxxopts::Options options(THORQ_APPLICATION_NAME, "Server for ThorQ - A application for long range collar control");
    options.add_options("", {
                            { "port",  "Port for the server to run at" },
                            { "config", "Configuration file for the server" }
                         });
    cxxopts::ParseResult result = options.parse(argc, argv);

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("ThorQ");
    QCoreApplication::setApplicationName(THORQ_VERSION_SERVER.toString());

    QCommandLineParser parser;
    parser.setApplicationDescription("");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOptions();

    parser.process(app);

#if PARSE_PORT
    if (!parser.isSet("port"))
    {
        return EXIT_FAILURE;
    }

    QVariant var(parser.value("port"));

    bool success = false;

    uint port = QVariant(parser.value("port")).toUInt(&success);

    if (!success || port > UINT16_MAX)
    {
        qDebug() << "Failed to parse port!";
        return EXIT_FAILURE;
    }
#else
    std::uint16_t port = 12345;
#endif

    if (!ThorQ::Connection::Initialize("database.db"))
    {
        return EXIT_FAILURE;
    }

    ThorQ::Account::NewAccount("yeet", "yeet");
    ThorQ::Account::NewAccount("yeet1", "yeet1");
    ThorQ::Account::NewAccount("yeet2", "yeet2");
    return EXIT_SUCCESS;

    if (!ThorQ::Server::Initialize())
    {
            qDebug() << "Failed to initialize Server!";
            return EXIT_FAILURE;
    }

    qDebug().noquote().nospace() << "Using" << ThorQ::Server::Version();

    ThorQ::Server* server = new ThorQ::Server();

    server->setup(port, SERVER_MAX_CONNECTIONS, THORQ_CHANNEL_COUNT, true);

    server->start();

    return app.exec();
}
