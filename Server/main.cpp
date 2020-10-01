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

#if defined(__GCC__) || defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif

#define ENET_IMPLEMENTATION
#include <enet.h>

#if defined(__GCC__) || defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#define SINGLETON_BASE
#include "singletons.h"
#include "account.h"
#include "instance.h"
#include "eventhandlers.h"

#define PARSE_PORT false
#define SERVER_MAX_CONNECTIONS 1024

bool enet_was_initialized = false;
std::atomic_bool runServer = true;

void exit_handler(int s)
{
    qDebug() << "Caught signal" << s;

    runServer.store(false);
}

void exitCleanup()
{
    if (!enet_was_initialized)
        return;

    if (g_server != nullptr)
    {
        for (ENetPeer* peer : g_peers)
            enet_peer_disconnect(peer, THORQ_DISCONNECT_REASON_SHUTDOWN_CLOSED);

        ENetEvent event;
        while (g_peers.size() != 0)
        {
            if (enet_host_service(g_server, &event, 0) > 0)
            {
                switch (event.type)
                {
                case ENET_EVENT_TYPE_CONNECT:
                    event.peer->data = nullptr;
                    enet_peer_disconnect_now(event.peer, THORQ_DISCONNECT_REASON_SHUTDOWN_CLOSED);
                    break;
                case ENET_EVENT_TYPE_RECEIVE:
                    enet_packet_destroy(event.packet);
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                {
                    if (event.peer->data != nullptr)
                    {
                        (reinterpret_cast<ThorQ::Instance*>(event.peer->data))->setPeer(nullptr);
                    }
                    enet_peer_reset(event.peer);
                    auto it = std::find(g_peers.begin(), g_peers.end(), event.peer);
                    if (it != g_peers.end())
                    {
                        g_peers.erase(it);
                    }
                    break;
                }
                case ENET_EVENT_TYPE_NONE:
                    break;
                }
            }
        }

        enet_host_destroy(g_server);
    }

    enet_deinitialize();

    qDebug() << "All clients are disconnected,\nIf i crash now, that is totally ok!";

    // This is very likely to crash the server, so do this last
    for (ThorQ::Instance* instance : g_sessions)
        delete instance;
}

int main(int argc, char** argv)
{
    signal(SIGINT, exit_handler);

    atexit(exitCleanup);

    ENetAddress address;
    address.host = ENET_HOST_ANY;
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
        address.port = i;
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
    address.port = THORQ_SERVER_PORT;
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

    if (enet_initialize() < 0)
    {
        qWarning() << "Failed to initialize ENet\n";
        return EXIT_FAILURE;
    }
    enet_was_initialized = true;

    qDebug() << QString("Using ENet-%1.%2.%3")
                .arg(ENET_VERSION_MAJOR)
                .arg(ENET_VERSION_MINOR)
                .arg(ENET_VERSION_PATCH);

    // Setup server
    g_server = enet_host_create(&address, SERVER_MAX_CONNECTIONS, 2, 0, 0); // two channels: communication(tcp), and commands(udp)

    if (g_server == nullptr)
    {
        qWarning() << "An error occurred while trying to create an ENet server host";
        return EXIT_FAILURE;
    }

    ENetEvent event;
    while (runServer.load()) {
        while (enet_host_service(g_server, &event, 0) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                g_peers.insert(event.peer);
                handleEventNewConnection(event.peer);
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                handleEventMessage(event.peer, event.packet);
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
            {
                handleEventDisconnect(event.peer);
                auto it = std::find(g_peers.begin(), g_peers.end(), event.peer);
                if (it != g_peers.end())
                {
                    g_peers.erase(it);
                }
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
            {
                handleEventTimeout(event.peer);
                auto it = std::find(g_peers.begin(), g_peers.end(), event.peer);
                if (it != g_peers.end())
                {
                    g_peers.erase(it);
                }
                break;
            }
            case ENET_EVENT_TYPE_NONE:
                break;
            }
        }
    }

    return EXIT_SUCCESS;
}
