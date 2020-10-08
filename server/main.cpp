#include <iostream>
#include <exception>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <atomic>
#include <algorithm>

#include <QDebug>
#include <QCoreApplication>
#include <QCommandLineParser>

#if __linux__
#include <unistd.h>
#endif

#include "server.h"
#include "messagedispatcher.h"

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

void exit_handler(int s)
{
    qDebug() << "Caught signal" << s;

    runServer.store(false);
}

void exitCleanup()
{
    g_server.cleanup();
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("ThorQ");
    QCoreApplication::setApplicationName(THORQ_VERSION_SERVER.toString());

    QCommandLineParser parser;
    parser.setApplicationDescription("Server for ThorQ - A application for long range collar control");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOptions({
                         { "port",  "Port for the server to run at" },
                         { "config", "Configuration file for the server" }
                      });

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

    if (!ThorQ::DataBase::Initialize("database.db"))
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

    ThorQ::Server* server = new ThorQ::Server(&app);

    server->setup(port, SERVER_MAX_CONNECTIONS, THORQ_CHANNEL_COUNT, true);

    server->start();

    return app.exec();
}
