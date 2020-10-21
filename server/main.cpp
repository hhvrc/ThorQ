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
#include "sqlite/connection.h"
#include "statistics.h"
#include "eventhandlers.h"
#include <thorq_message.h>

#define PARSE_PORT false
#define SERVER_MAX_CONNECTIONS 1024

std::atomic_bool runServer = true;

bool InitializeDB(const char* path) noexcept
{
    ThorQ::SQLite::Connection con(path, ThorQ::SQLite::Connection::CREATE | ThorQ::SQLite::Connection::READWRITE);

    return con.execute("CREATE TABLE IF NOT EXISTS system_ids(db_id INTEGER PRIMARY KEY AUTOINCREMENT, system_id TEXT NOT NULL UNIQUE, banned_at DATETIME, registered_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)") // Unique SystemID of a cmoputer
        && con.execute("CREATE TABLE IF NOT EXISTS auth_tokens(db_id INTEGER PRIMARY KEY AUTOINCREMENT, auth_token TEXT NOT NULL UNIQUE, system_id INTEGER NOT NULL, account_id INTEGER NOT NULL, created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)") // Authentication Token generated at login
        && con.execute("CREATE TABLE IF NOT EXISTS accounts(db_id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT NOT NULL UNIQUE, password_hash TEXT NOT NULL, authority INTEGER NOT NULL DEFAULT 0, last_login DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, deleted_at DATETIME)")
        && con.execute("CREATE TABLE IF NOT EXISTS relationships(db_id INTEGER PRIMARY KEY AUTOINCREMENT, uuid TEXT NOT NULL UNIQUE, source INTEGER NOT NULL REFERENCES accounts, target INTEGER NOT NULL REFERENCES account, status INTEGER NOT NULL, authority INTEGER, created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)")
        && con.execute("CREATE TABLE IF NOT EXISTS systemid_account_map(systemid_id INTEGER NOT NULL, account_id INTEGER NOT NULL, established_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)")
        && con.execute("CREATE TABLE IF NOT EXISTS userLog(timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, system_id INTEGER, account_id INTEGER, info TEXT NOT NULL)");
}
void exit_handler(int s)
{
    char buf[64];
#ifdef _WIN32
    strerror_s(buf, sizeof(buf), s);
#else
    strerror_r(s, buf, sizeof(buf));
#endif

    printf("Caught signal %s\n", buf);

    runServer.store(false);
}

void exitCleanup()
{
    // g_server->cleanup();

    delete g_server;
}

int main(int argc, char** argv)
{
    printf("ThorQ Server %s\n", THORQ_VERSION_SERVER.toString().c_str());
    printf("Using link %s\n", THORQ_VERSION_LINK.toString().c_str());
    printf("Expecting client %s\n", THORQ_VERSION_CLIENT.toString().c_str());

    cxxopts::Options options(THORQ_APPLICATION_NAME, "Server for ThorQ - A application for long range collar control");
    options.add_options()
            ( "p,port", "Port for the server to run at", cxxopts::value<std::uint16_t>())
            ( "c,conf", "Configuration file for the server", cxxopts::value<std::string>())
            ;
    cxxopts::ParseResult result = options.parse(argc, argv);
#if PARSE_PORT
    std::uint16_t port = result["port"].as<std::uint16_t>();
#else
    std::uint16_t port = 12345;
#endif

    if (!InitializeDB("database.db"))
    {
        return EXIT_FAILURE;
    }

    ThorQ::Account::NewAccount("yeet", "yeet");
    ThorQ::Account::NewAccount("yeet1", "yeet1");
    ThorQ::Account::NewAccount("yeet2", "yeet2");
    return EXIT_SUCCESS;

    if (!ThorQ::Server::Initialize())
    {
            printf("Failed to initialize Server!\n");
            return EXIT_FAILURE;
    }

    g_server = new ThorQ::Server(port, 1024, THORQ_CHANNEL_COUNT, true);

    if (!g_server->ready())
    {
            printf("Failed to start Server!\n");
            return EXIT_FAILURE;
    }

    while (runServer) { std::this_thread::sleep_for(std::chrono::milliseconds(500)); }
}
