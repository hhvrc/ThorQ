#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

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

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
