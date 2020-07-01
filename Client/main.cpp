#include <QDebug>
#include <QGuiApplication>

#include <enet.h>

#include "client.h"

int main(int argc, char** argv)
{
    // TODO: make GUI the main thread, and Networking a seperate thread
    // TODO: customize GUI
    // TODO: enable support for SteamVR
    // TODO: Add pre-encryption flag that signalises if connection is encrypted or not so clients can re-authenticate

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    // Initialize ENet
    if (enet_initialize() < 0)
    {
        printf("Failed to initialize ENet");
        exit(EXIT_FAILURE);
    }
    atexit(enet_deinitialize);
    qDebug().noquote() << "Using" << Client::Version();



    return app.exec();
}
