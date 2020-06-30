#include <iostream>

#define ENET_IMPLEMENTATION
#include <enet.h>

#include <QDebug>

#include <enums.h>
#include <crypto.h>

#define SERVER_HOSTNAME "::1"
#define SERVER_PORT     12345

#define DISCONNECT_ERROR 0x00000001

#include "client.h"

#include <chrono>



#include <QApplication>

int main(int argc, char** argv)
{
	QApplication application(argc, argv);
	application.setApplicationName("ThorQ Client");

	// TODO: make GUI the main thread, and Networking a seperate thread
	// TODO: customize GUI
	// TODO: enable support for SteamVR
	// TODO: Add pre-encryption flag that signalises if connection is encrypted or not so clients can re-authenticate

	if (enet_initialize() < 0)
	{
		printf("Failed to initialize ENet");
		exit(EXIT_FAILURE);
	}
	atexit(enet_deinitialize);

	qDebug().noquote() << "Using" << Client::Version();

	Client* cli = Client::NewClient("localhost", 12345);

	if (cli == nullptr)
	{
		qDebug() << "Failed!";
		return EXIT_FAILURE;
	}

	delete cli;

	application.exec();

	return EXIT_SUCCESS;
}
