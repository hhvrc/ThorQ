#include <iostream>
#include <exception>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <atomic>
#include <algorithm>

#include <QtSql>
#include <QDebug>

#if __linux__
#include <unistd.h>
#endif

#include "server.h"
#include "parser.h"

#define SINGLETON_BASE
#include "singletons.h"
#include "account.h"
#include "instance.h"
#include "statistics.h"
#include "eventhandlers.h"
#include <thorq_message.h>

#define PARSE_PORT false
#define SERVER_MAX_CONNECTIONS 1024

std::atomic_bool runServer = true;

void exit_handler(int s)
{
    qDebug() << "Caught signal" << s;

    runServer.store(false);
}

void exitCleanup()
{


    qDebug() << "All clients are disconnected,\nIf i crash now, that is totally ok!";

    // This is very likely to crash the server, so do this last
    for (ThorQ::Instance* instance : g_sessions)
        delete instance;
}

int main(int argc, char** argv)
{
    signal(SIGINT, exit_handler);

    atexit(exitCleanup);


    std::uint16_t port = 0;
#if PARSE_PORT
    if (argc < 2)
    {
        qWarning() << "Please provide a port";
        return EXIT_FAILURE;
    }
    else if (argc > 3)
    {
        qWarning() << "Too many arguments";
        return EXIT_FAILURE;
    }

    try {
        int i = std::stoi(argv[1]);
        if (i < 1 || i > UINT16_MAX)
        {
            qWarning() << "Port must be in the range of 1-65535";
            return EXIT_FAILURE;
        }
        port = i;
    } catch (std::invalid_argument) {
        qWarning() << "Port must be a number";
        return EXIT_FAILURE;
    } catch (std::out_of_range) {
        qWarning() << "Port must be in the range of 1-65535";
        return EXIT_FAILURE;
    } catch (const std::exception& ex) {
        qWarning() << "Exception occured while parsing argument:\n" << ex.what();
        return EXIT_FAILURE;
    }
#else
    (void)argc;
    (void)argv;
    port = THORQ_SERVER_PORT;
#endif
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("database.db");
        if (!db.open())
        {
            qWarning() << "Failed to open database:" << db.lastError();
            return EXIT_FAILURE;
        }

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
                "established_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)");   // Unique SystemID of a cmoputer
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

        db.close();

        ThorQ::Account::NewAccount("yeet", "yeet", "yeet");
        ThorQ::Account::NewAccount("yeet1", "yeet1", "yeet1");
        ThorQ::Account::NewAccount("yeet2", "yeet2", "yeet2");
        return EXIT_SUCCESS;
    }


    qDebug().noquote().nospace()
            << "Using " << ThorQ::Server::version();

    ThorQ::Parser* parser = new ThorQ::Parser();
    ThorQ::Server* server = new ThorQ::Server();

    QObject::connect(server, &ThorQ::Server::enetEvent, parser, &ThorQ::Parser::handleEvent);

    if (!server->setup(port, SERVER_MAX_CONNECTIONS, THORQ_CHANNEL_COUNT))
    {
        return EXIT_FAILURE;
    }

    server->setMaxPacketSize(THORQ_MESSAGE_LEN);

    server->start(QThread::HighPriority);

    return EXIT_SUCCESS;
}
