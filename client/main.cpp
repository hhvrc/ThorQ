#include <QApplication>
#include <QCoreApplication>
#include <QLabel>
#include <QIcon>
#include <QInputDialog>
#include <QRegExp>
#include <QDebug>
#include <QDir>
#include <QTranslator>
#include <QMessageBox>
#include <QVBoxLayout>

#include "loginwidget.h"
#include "mainwidget.h"
#include "collar/serial.h"
#include "apiclient.h"
#include "vr/openvroverlaycontroller.h"

Q_DECLARE_METATYPE(ConnectionStatus)
Q_DECLARE_METATYPE(ProcessStatus)
Q_DECLARE_METATYPE(CryptoLinkStatus)
Q_DECLARE_METATYPE(THORQ_STATE_HWID)
Q_DECLARE_METATYPE(THORQ_STATE_LOGIN)

#define COMTEST 1

#include <constants.h>
#include <encoding.h>

#include <filesystem>

std::uint16_t i = 0;
int main(int argc, char** argv)
{
    // TODO: customize GUI
    // TODO: enable support for SteamVR
    QCoreApplication::addLibraryPath(QDir::currentPath().append("/libs/"));

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setApplicationName(THORQ_APPLICATION_NAME);
    QCoreApplication::setApplicationVersion(ThorQ::ClientVersion.toString().c_str());
    QCoreApplication::setOrganizationName(THORQ_ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(THORQ_ORGANIZATION_DOMAIN);

    QApplication app(argc, argv);
#if COMTEST
#else
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
#endif
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
    LoginWidget loginWidget;
    MainWidget mainWidget;
    ThorQ::ApiClient* apiClient = new ThorQ::ApiClient(&app);

    QMessageBox errorBox(&loginWidget);
    errorBox.setIcon(QMessageBox::Critical);
    errorBox.setWindowTitle("error");

    QMessageBox warningBox(&loginWidget);
    warningBox.setIcon(QMessageBox::Warning);
    errorBox.setWindowTitle("warning");

    QObject::connect(apiClient, &ThorQ::ApiClient::connectionStatusChanged, &loginWidget, &LoginWidget::setConnectionStatus);
    QObject::connect(apiClient, &ThorQ::ApiClient::errorOccured, &errorBox, &QMessageBox::setText);
    QObject::connect(apiClient, &ThorQ::ApiClient::errorOccured, &errorBox, &QMessageBox::show);
/*
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
*/
    loginWidget.show();
    apiClient->netConnect("localhost", 12345);

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
#else
    ThorQ::VR::Shutdown();
#endif
    return retval;
}
