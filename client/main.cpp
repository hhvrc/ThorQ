#include <QApplication>
#include <QCoreApplication>
#include <QLabel>
#include <QIcon>
#include <QInputDialog>
#include <QRegExp>
#include <QDebug>
#include <QTranslator>
#include <QMessageBox>

#include <enet.h>

#include "loginwidget.h"
#include "mainwidget.h"
#include "collar/serial.h"
#include "networking/host.h"
#include "networking/connectionhandler.h"
#include "vr/openvroverlaycontroller.h"

Q_DECLARE_METATYPE(THORQ_STATE_CONNECTION)
Q_DECLARE_METATYPE(THORQ_STATE_CRYPTO)
Q_DECLARE_METATYPE(THORQ_STATE_HWID)
Q_DECLARE_METATYPE(THORQ_STATE_LOGIN)

#define COMTEST 0

#include <constants.h>
#include <crypto.h>
#include <thorq_message.h>

#include <QVBoxLayout>

#include <thorq_message.h>

#include <serverhandler.h>

std::uint16_t i = 0;
int main(int argc, char** argv)
{
    qRegisterMetaType<QSharedPointer<ThorQ::Networking::Message>>("ThorQNetworkingMessage");

    // TODO: customize GUI
    // TODO: enable support for SteamVR

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setApplicationName(THORQ_APPLICATION_NAME);
    QCoreApplication::setApplicationVersion(THORQ_VERSION_CLIENT.toString().c_str());
    QCoreApplication::setOrganizationName(THORQ_ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(THORQ_ORGANIZATION_DOMAIN);

    QApplication app(argc, argv);

    if (!ThorQ::VR::Initialize()) {
        qDebug() << "Failed to init VR";
        return EXIT_FAILURE;
    }

    if (ThorQ::VR::IsManifestInstalled()) {
        ThorQ::VR::RemoveManifest();
    }

    if (!ThorQ::VR::CreateManifest()) {
        qDebug() << "Failed to create VR manifest files";
        return EXIT_FAILURE;
    }

    if (!ThorQ::VR::InstallManifest()) {
        qDebug() << "Failed to install VR manifest files";
        return EXIT_FAILURE;
    }

    QString stylesheet;

    const char* fileName;
    if (QFile::exists("stylesheet.css"))
    { fileName = "stylesheet.css"; }
    else
    { fileName = ":/stylesheet.css"; }

    QFile file(fileName);
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream stream(&file);
        stylesheet = stream.readAll();
    }

    app.setStyleSheet(stylesheet);
    app.setDesktopFileName(THORQ_APPLICATION_NAME);
    app.setWindowIcon(QIcon(":/shockGrey.ico"));
    //app.setQuitOnLastWindowClosed(false);
#if COMTEST
    // Initialize ENet
    if (!ThorQ::Networking::Host::Initialize())
    {
        qDebug() << "Failed to initialize networking";
        return EXIT_FAILURE;
    }

    LoginWidget loginWidget;
    MainWidget mainWidget;
    ThorQ::Networking::Host* host = ThorQ::Networking::Host::CreateHost(8, &app);

    if (host == nullptr) {
        qDebug() << "Failed to create host";
        return EXIT_FAILURE;
    }

    ThorQ::ServerHandler* handler = new ThorQ::ServerHandler(&app);
    QObject::connect(handler, &ThorQ::ServerHandler::requestConnect, host, &ThorQ::Networking::Host::connect);

    host->connect(THORQ_SERVER_HOSTNAME, THORQ_SERVER_PORT, (std::uint8_t)THORQ_CHANNEL::_MAX, handler->connectionHandler());


/*
    QMessageBox errorBox(&loginWidget);
    errorBox.setIcon(QMessageBox::Critical);
    errorBox.setWindowTitle("error");

    QMessageBox warningBox(&loginWidget);
    warningBox.setIcon(QMessageBox::Warning);
    errorBox.setWindowTitle("warning");

    QObject::connect(cli, &ThorQ::Networking::Client::rttChanged, &mainWidget, &MainWidget::setConnectionPing);
    QObject::connect(cli, &ThorQ::Networking::Client::LoginStateChanged, &mainWidget, &MainWidget::setLoginState);
    QObject::connect(cli, &ThorQ::Networking::Client::SessionStateChanged, &mainWidget, &MainWidget::setSessionState);

    QObject::connect(cli, &ThorQ::Networking::Client::RttChanged, &loginWidget, &LoginWidget::setConnectionPing);
    QObject::connect(cli, &ThorQ::Networking::Client::ConnectionStateChanged, &loginWidget, &LoginWidget::setConnectionState);
    QObject::connect(cli, &ThorQ::Networking::Client::CryptoStateChanged, &loginWidget, &LoginWidget::setCryptoState);
    QObject::connect(cli, &ThorQ::Networking::Client::AuthStateChanged, &loginWidget, &LoginWidget::setHwidState);
    QObject::connect(cli, &ThorQ::Networking::Client::LoginStateChanged, &loginWidget, &LoginWidget::setLoginState);

    QObject::connect(cli, &ThorQ::Networking::Client::Error, &errorBox, &QMessageBox::setText);
    QObject::connect(cli, &ThorQ::Networking::Client::Error, &errorBox, &QWidget::show);
    //QObject::connect(cli, &Client::Error, [&](){ app.setQuitOnLastWindowClosed(true); loginWidget.hide(); mainWidget.hide(); warningBox.hide(); });

    QObject::connect(cli, &Client::Warning, &warningBox, &QMessageBox::setText);
    QObject::connect(cli, &Client::Warning, &warningBox, &QWidget::show);

    QObject::connect(&loginWidget, &LoginWidget::usernameEntered, cli, &Client::Login);

    QObject::connect(cli, &Client::userUpdate, &mainWidget, &MainWidget::updateUser);
    QObject::connect(cli, &Client::UserOffline, &mainWidget, &MainWidget::removeUser);
    QObject::connect(&mainWidget, &MainWidget::logoutButtonClicked, cli, &Client::Logout);

    loginWidget.show();
*/
#else
    QPixmap pix(":/uwu.png");
    QLabel lab;
    lab.setPixmap(pix);

    OpenVROverlayController* ovr = new OpenVROverlayController(&app);

    ovr->init();
    ovr->setWidget(&lab);
    QObject::connect(ovr, &OpenVROverlayController::vrQuit, ovr, &QObject::deleteLater);
#endif

    int retval = app.exec();

#if COMTEST
    delete host;
    ThorQ::Networking::Host::DeInitialize();
#else
    ThorQ::VR::Shutdown();
#endif
    return retval;
}
