#include <QDebug>
#include <QApplication>
#include <QCoreApplication>
#include <QLabel>
#include <QIcon>

#include <enet.h>
#include "client.h"
#include "serial.h"
#include "loginwidget.h"
#include "openvroverlaycontroller.h"

Q_DECLARE_METATYPE(thorq_connection_state_t)
Q_DECLARE_METATYPE(thorq_login_state_t)

#define COMTEST 1

int main(int argc, char** argv)
{
	qRegisterMetaType<thorq_connection_state_t>("ThorqConnectionState");
	qRegisterMetaType<thorq_login_state_t>("ThorqLoginState");

	// TODO: make GUI the main thread, and Networking a seperate thread
	// TODO: customize GUI
	// TODO: enable support for SteamVR
	// TODO: Add pre-encryption flag that signalises if connection is encrypted or not so clients can re-authenticate

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
    app.setApplicationName("ThorQ");
    app.setDesktopFileName("ThorQ");
    app.setApplicationVersion(THORQ_VERSION_CLIENT.to_string().c_str());
    app.setWindowIcon(QIcon(":/shockGrey.ico"));

	// Initialize ENet
	if (enet_initialize() < 0)
	{
		printf("Failed to initialize ENet");
		exit(EXIT_FAILURE);
	}
	qDebug().noquote() << "Using" << Client::Version();

	LoginWidget e;
	e.show();

	Client* cli = Client::NewClient();
	QObject::connect(cli, &Client::ConnectionStateChanged, &e, &LoginWidget::SetConnectionState);
	QObject::connect(cli, &Client::LoginStateChanged, &e, &LoginWidget::SetLoginState);
	QObject::connect(cli, &Client::PingChanged, &e, &LoginWidget::SetConnectionPing);

	QObject::connect(&e, &LoginWidget::LoginRequest, [&](const QString& username)
	{
	   qDebug() << username;
	});

	cli->Connect("www.potato.tech", 12345);
#else
	QPixmap pix(":/uwu.png");
	QLabel lab;
	lab.setPixmap(pix);

	OpenVROverlayController::SharedInstance()->Init();
	OpenVROverlayController::SharedInstance()->SetWidget(&lab);
#endif

	int retval = app.exec();

	enet_deinitialize();

	return retval;
}
