#include <QDebug>
#include <QApplication>
#include <QCoreApplication>

#include <enet.h>
#include "client.h"
#include "loginwidget.h"
#include "openvroverlaycontroller.h"

int main(int argc, char** argv)
{
    // TODO: make GUI the main thread, and Networking a seperate thread
    // TODO: customize GUI
    // TODO: enable support for SteamVR
    // TODO: Add pre-encryption flag that signalises if connection is encrypted or not so clients can re-authenticate

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);

    QString stylesheet;
    QFile file("stylesheet.css");
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream stream(&file);
        stylesheet = stream.readAll();
    }
    app.setStyleSheet(stylesheet);

    // Initialize ENet
    if (enet_initialize() < 0)
    {
        printf("Failed to initialize ENet");
        exit(EXIT_FAILURE);
    }
    atexit(enet_deinitialize);
    qDebug().noquote() << "Using" << Client::Version();

    LoginWidget e;
    e.show();

    //OpenVROverlayController::SharedInstance()->Init();
    //OpenVROverlayController::SharedInstance()->SetWidget(&label);

    return app.exec();
}
