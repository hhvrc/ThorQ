#include <QApplication>
#include <QCoreApplication>
#include <QLabel>
#include <QIcon>

#include <log.h>
#include <enet.h>

#include "client.h"
#include "serial.h"
#include "loginwidget.h"
#include "openvroverlaycontroller.h"

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

	// Initialize ENet
	if (enet_initialize() < 0)
	{
		printf("Failed to initialize ENet");
        return EXIT_FAILURE;
	}
	thorq_debug_fmt("Using %s", Client::Version().toStdString().c_str())

    LoginWidget e;
    e.show();

    Client* cli = Client::NewClient();
	QObject::connect(cli, &Client::ConnectionStateChanged, &e, &LoginWidget::SetConnectionState);
    QObject::connect(cli, &Client::CryptoStateChanged, &e, &LoginWidget::SetCryptoState);
    QObject::connect(cli, &Client::AuthStateChanged, &e, &LoginWidget::SetAuthState);
	QObject::connect(cli, &Client::LoginStateChanged, &e, &LoginWidget::SetLoginState);
	QObject::connect(cli, &Client::PingChanged, &e, &LoginWidget::SetConnectionPing);

	QObject::connect(&e, &LoginWidget::LoginRequest, [&](const QString& username)
	{
	   qDebug() << username;
	});
    QObject::connect(cli, &Client::RequestingRegistrationKey, [&]()
    {
    });

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
