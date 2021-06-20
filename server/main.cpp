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

#include "apiserver.h"

#define PARSE_PORT false

std::atomic_bool runServer = true;

bool InitializeDB(const char* path) noexcept
{
    auto con = SQLite::Connection::OpenConnection(path, SQLite::Connection::CREATE | SQLite::Connection::READWRITE);

    using namespace std::literals;
    return con->execute("CREATE TABLE IF NOT EXISTS accounts("
                "account_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "uuid VARCHAR(26) NOT NULL UNIQUE,"
                "username VARCHAR(32) NOT NULL UNIQUE,"
                "password_id INTEGER NOT NULL REFERENCES passwords,"
                "email_address VARCHAR(256),"
                "image_id INTEGER NOT NULL REFERENCES images,"
                "created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
                "updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
                "banned_at DATETIME,"
                "deleted_at DATETIME"
                ")"sv)

        && con->execute("CREATE TABLE IF NOT EXISTS passwords("
                "password_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "salt BLOB NOT NULL,"
                "hash BLOB,"
                "ops_limit INTEGER NOT NULL DEFAULT 4,"
                "mem_limit INTEGER NOT NULL DEFAULT 1073741824,"
                "algorithm INTEGER NOT NULL DEFAULT 2"
                ")"sv)

        && con->execute("CREATE TABLE IF NOT EXISTS account_systems("
                "account_id INTEGER REFERENCES accounts,"
                "system_id INTEGER REFERENCES systems,"
                "auth_token VARCHAR(64)," // Authtoken for non-credentials login, optionally created at login
                "last_login DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
                "connected_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
                "PRIMARY KEY(account_id, system_id)"
                ")"sv)

        && con->execute("CREATE TABLE IF NOT EXISTS systems("
                "system_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "hardware_id VARCHAR(280) NOT NULL UNIQUE," // Unique HardwareID of a computer
              //"application_build_id INTEGER NOT NULL REFERENCES application_builds,"
                "registered_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP"
                ")"sv)

        && con->execute("CREATE TABLE IF NOT EXISTS system_types("
                "application_build_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "os_name VARCHAR(20) NOT NULL,"
                "is_mobile BOOLEAN NOT NULL"
                ")"sv)

        && con->execute("CREATE TABLE IF NOT EXISTS account_roles("
                "account_id INTEGER REFERENCES accounts,"
                "role_id INTEGER REFERENCES roles,"
                "assigned_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
                "PRIMARY KEY(account_id, role_id)"
                ")"sv)

        && con->execute("CREATE TABLE IF NOT EXISTS roles("
                "role_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "name VARCHAR(30) NOT NULL UNIQUE"
                ")"sv)

        && con->execute("CREATE TABLE IF NOT EXISTS images("
                "image_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "uuid VARCHAR(26) NOT NULL UNIQUE," // Image path will be images/{uuid}.png
                "uploaded_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
                "uploader_id INTEGER REFERENCES accounts"
                ")"sv)

        && con->execute("CREATE TABLE IF NOT EXISTS relationships("
                "source INTEGER NOT NULL REFERENCES accounts,"
                "target INTEGER NOT NULL REFERENCES accounts,"
                "nickname TEXT,"
                "friendship INTEGER NOT NULL,"
                "is_muted BOOLEAN NOT NULL,"
                "is_blocked BOOLEAN NOT NULL,"
                "auto_accept BOOLEAN NOT NULL,"
                "notify_online BOOLEAN NOT NULL,"
                "created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
                "PRIMARY KEY (source, target)"
                ") WITHOUT ROWID"sv)

            && con->execute("CREATE TABLE IF NOT EXISTS friendrequests("
                    "source INTEGER NOT NULL REFERENCES accounts,"
                    "target INTEGER NOT NULL REFERENCES accounts,"
                    "requested_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
                    "PRIMARY KEY (source, target)"
                    ") WITHOUT ROWID"sv);
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
        fmt::print("Failed to initialize database\n");
        return EXIT_FAILURE;
    }
    fmt::print("Initialized database\n");

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
