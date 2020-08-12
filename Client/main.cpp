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

#include "client.h"
#include "serial.h"
#include "loginwidget.h"
#include "openvroverlaycontroller.h"
#include "mainwidget.h"

Q_DECLARE_METATYPE(thorq_connection_state_t)
Q_DECLARE_METATYPE(thorq_crypto_state_t)
Q_DECLARE_METATYPE(thorq_auth_state_t)
Q_DECLARE_METATYPE(thorq_login_state_t)

#define COMTEST 1

#include <crypto.h>
#include <thorq_message.h>
#include <thorq_payload.h>
#include <thorq_payload_crypto.h>


int main(int argc, char** argv)
{
	qRegisterMetaType<thorq_connection_state_t>("ThorqConnectionState");
    qRegisterMetaType<thorq_crypto_state_t>("ThorqCryptoState");
    qRegisterMetaType<thorq_auth_state_t>("ThorqAuthState");
	qRegisterMetaType<thorq_login_state_t>("ThorqLoginState");

	// TODO: customize GUI
    // TODO: enable support for SteamVR

	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication app(argc, argv);
#if COMTEST
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
	app.setApplicationName(THORQ_APPLICATION_NAME);
	app.setDesktopFileName(THORQ_APPLICATION_NAME);
	app.setApplicationVersion(THORQ_VERSION_CLIENT.to_string().c_str());
	app.setWindowIcon(QIcon(":/shockGrey.ico"));
    //app.setQuitOnLastWindowClosed(false);

	// Initialize ENet
	if (enet_initialize() < 0)
	{
		printf("Failed to initialize ENet");
        return EXIT_FAILURE;
	}

	LoginWidget loginWidget;
    MainWidget mainWidget;
	Client* cli = Client::NewClient();

	QMessageBox errorBox(&loginWidget);
	errorBox.setIcon(QMessageBox::Critical);
	errorBox.setWindowTitle("error");

	QMessageBox warningBox(&loginWidget);
	warningBox.setIcon(QMessageBox::Warning);
	errorBox.setWindowTitle("warning");

	QObject::connect(cli, &Client::PingChanged, &mainWidget, &MainWidget::setConnectionPing);
	QObject::connect(cli, &Client::LoginStateChanged, &mainWidget, &MainWidget::setLoginState);
	QObject::connect(cli, &Client::SessionStateChanged, &mainWidget, &MainWidget::setSessionState);

	QObject::connect(cli, &Client::PingChanged, &loginWidget, &LoginWidget::setConnectionPing);
    QObject::connect(cli, &Client::ConnectionStateChanged, &loginWidget, &LoginWidget::setConnectionState);
    QObject::connect(cli, &Client::CryptoStateChanged, &loginWidget, &LoginWidget::setCryptoState);
    QObject::connect(cli, &Client::AuthStateChanged, &loginWidget, &LoginWidget::setAuthState);
	QObject::connect(cli, &Client::LoginStateChanged, &loginWidget, &LoginWidget::setLoginState);

    QObject::connect(cli, &Client::Error, &errorBox, &QMessageBox::setText);
	QObject::connect(cli, &Client::Error, &errorBox, &QWidget::show);
	//QObject::connect(cli, &Client::Error, [&](){ app.setQuitOnLastWindowClosed(true); loginWidget.hide(); mainWidget.hide(); warningBox.hide(); });

	QObject::connect(cli, &Client::Warning, &warningBox, &QMessageBox::setText);
	QObject::connect(cli, &Client::Warning, &warningBox, &QWidget::show);

    QObject::connect(&loginWidget, &LoginWidget::regkeyEntered, cli, &Client::SetRegistrationKey);
	QObject::connect(&loginWidget, &LoginWidget::usernameEntered, cli, &Client::Login);
	QObject::connect(&mainWidget, &MainWidget::logoutButtonClicked, cli, &Client::Logout);

	loginWidget.show();

	cli->Connect(THORQ_SERVER_HOSTNAME, THORQ_SERVER_PORT);

#else
	QPixmap pix(":/uwu.png");
	QLabel lab;
	lab.setPixmap(pix);

    OpenVROverlayController* ovr = new OpenVROverlayController(&app);
    ovr->Init();
    ovr->SetWidget(&lab);
    QObject::connect(ovr, &OpenVROverlayController::VrExited, ovr, &QObject::deleteLater);
#endif

	int retval = app.exec();

#if COMTEST
    delete cli;
    enet_deinitialize();
#endif
    return retval;
}
