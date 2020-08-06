#include <QApplication>
#include <QCoreApplication>
#include <QLabel>
#include <QIcon>
#include <QInputDialog>
#include <QRegExp>

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

	LoginWidget loginWidget;
	loginWidget.moveToThread(app.thread());
	loginWidget.show();

	Client* cli = Client::NewClient();
	QObject::connect(cli, &Client::ConnectionStateChanged, &loginWidget, &LoginWidget::SetConnectionState);
	QObject::connect(cli, &Client::CryptoStateChanged, &loginWidget, &LoginWidget::SetCryptoState);
	QObject::connect(cli, &Client::AuthStateChanged, &loginWidget, &LoginWidget::SetAuthState);
	QObject::connect(cli, &Client::LoginStateChanged, &loginWidget, &LoginWidget::SetLoginState);
	QObject::connect(cli, &Client::PingChanged, &loginWidget, &LoginWidget::SetConnectionPing);

	QInputDialog* dialog = new QInputDialog(&loginWidget);
	dialog->setWindowTitle("Please provide a registration key");
	dialog->setLabelText("Registration key:");
	QObject::connect(cli, &Client::RequestingRegistrationKey, dialog, &QWidget::show);
	QObject::connect(dialog, &QInputDialog::textValueSelected, [&](const QString& input){ cli->SetRegistrationKey(input); });

	QObject::connect(&loginWidget, &LoginWidget::LoginRequest, [&](const QString& username)
	{
	   qDebug() << username;
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
