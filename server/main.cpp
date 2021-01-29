#include <csignal>
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

#include <fmt/core.h>
#include <cxxopts.hpp>
#include <lsql/connection.h>

#include <constants.h>
#include <networking/server.h>

#include "apiserver.h"

#define PARSE_PORT false

std::atomic_bool runServer = true;

bool InitializeDB(const char* path) noexcept
{
    LSql::Connection con(path, LSql::Connection::CREATE | LSql::Connection::READWRITE);

    return con.execute("CREATE TABLE IF NOT EXISTS system_ids(db_id INTEGER PRIMARY KEY AUTOINCREMENT, system_id TEXT NOT NULL UNIQUE, banned_at DATETIME, registered_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)") // Unique SystemID of a cmoputer
        && con.execute("CREATE TABLE IF NOT EXISTS auth_tokens(db_id INTEGER PRIMARY KEY AUTOINCREMENT, auth_token TEXT NOT NULL UNIQUE, system_id INTEGER NOT NULL, account_id INTEGER NOT NULL, created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)") // Authentication Token generated at login
        && con.execute("CREATE TABLE IF NOT EXISTS accounts(db_id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT NOT NULL UNIQUE, password_hash TEXT NOT NULL, authority INTEGER NOT NULL DEFAULT 0, last_login DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, deleted_at DATETIME)")
        && con.execute("CREATE TABLE IF NOT EXISTS relationships(source INTEGER NOT NULL REFERENCES accounts, target INTEGER NOT NULL REFERENCES accounts, uuid TEXT NOT NULL UNIQUE, status INTEGER NOT NULL, authority INTEGER NOT NULL, created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, PRIMARY KEY (source, target)) WITHOUT ROWID")
        && con.execute("CREATE TABLE IF NOT EXISTS systemid_account_map(systemid_id INTEGER NOT NULL, account_id INTEGER NOT NULL, established_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)")
        && con.execute("CREATE TABLE IF NOT EXISTS userLog(timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, system_id INTEGER, account_id INTEGER, info TEXT NOT NULL)");
}
void exit_handler(int s)
{
    const char* str;
    switch (s) {
    case SIGINT:
        str = "interrupt";
        break;
    case SIGTERM:
        str = "terminate";
        break;
    case SIGABRT:
        str = "abort";
        break;
    case SIGSEGV:
        str = "segmentation fault";
        break;
    case SIGFPE:
        str = "erroneous arithmetic operation";
        break;
    default:
        str = "unknown";
        break;
    }

    fmt::print("\nCaught {} signal\n", str);

    runServer.store(false);
}

int main(int argc, char** argv)
{
    cxxopts::Options options(THORQ_APPLICATION_NAME, "Server for ThorQ - A application for long range collar control");
    options.add_options()
            ( "h,help", "Show this menu" )
            ( "p,port", "Port for the server to run at", cxxopts::value<std::uint16_t>())
            ( "c,conf", "Configuration file for the server", cxxopts::value<std::string>())
            ;
    cxxopts::ParseResult result = options.parse(argc, argv);

    if (result["help"].count() != 0)
    {
        fmt::print(options.help());
        return EXIT_SUCCESS;
    }

    fmt::print("Using server version     [{}]\n", ThorQ::ServerVersion.toString());
    fmt::print("Using link version       [{}]\n", ThorQ::LinkVersion.toString());
    fmt::print("Expecting client version [{}]\n", ThorQ::ClientVersion.toString());

#if PARSE_PORT
    cxxopts::OptionValue portValue = result["port"];

    if (portValue.count() == 0)
    {
        fmt::print("Please provide a port for the server to run at, or a config file to read this from\n");
        return EXIT_SUCCESS;
    }

    std::uint16_t port = portValue.as<std::uint16_t>();
#else
    std::uint16_t port = THORQ_SERVER_PORT;
#endif
    // Initialize database
    if (!InitializeDB("database.db"))
    {
        return EXIT_FAILURE;
    }

    // Create server
    std::shared_ptr<ThorQ::ApiServer> apiServer;
    try
    {
        apiServer = std::make_shared<ThorQ::ApiServer>(port);
    }
    catch (std::exception& ex)
    {
        fmt::print("Failed to create server instance: {}\n", ex.what());
        return EXIT_FAILURE;
    }
    catch (...)
    {
        fmt::print("Failed to create server instance: Unknown error\n");
        return EXIT_FAILURE;
    }

    // Start server
    if (!apiServer->start(std::thread::hardware_concurrency() * 2))
    {
        fmt::print("Failed to start Server!\n");
        return EXIT_FAILURE;
    }

    apiServer->status();

    // Register signal handlers
    std::signal(SIGINT, exit_handler);
    std::signal(SIGTERM, exit_handler);
    std::signal(SIGABRT, exit_handler);
    std::signal(SIGSEGV, exit_handler);
    std::signal(SIGFPE, exit_handler);

    // Hold
    while (runServer) { std::this_thread::sleep_for(std::chrono::milliseconds(100)); }

    // Stop server
    apiServer->stop();

    return EXIT_SUCCESS;
}
