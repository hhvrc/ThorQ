#include <iostream>
#include <exception>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <atomic>
#if __linux__
#include <unistd.h>
#endif

#if defined(__GCC__) || defined(__GNUC__) || defined(__clang__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wextra"
	#pragma GCC diagnostic ignored "-Wpedantic"
	#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif

#define ENET_IMPLEMENTATION
#include <enet.h>

#if defined(__GCC__) || defined(__GNUC__) || defined(__clang__)
	#pragma GCC diagnostic pop
#endif

#define SINGLETON_BASE
#include "singletons.h"
#include "instancemap.h"
#include "eventhandlers.h"

#define PARSE_PORT false
#define SERVER_PORT 12345
#define SERVER_MAX_CONNECTIONS 1024

bool enet_was_initialized = false;
std::atomic_bool runServer = true;

void exit_handler(int s)
{
	printf("Caught signal %i!\n", s);
	fflush(stdout);

	runServer.store(false);
}

void exitCleanup()
{
	if (enet_was_initialized)
		enet_deinitialize();

	if (registeredInstances != nullptr)
		delete registeredInstances;

	if (server != nullptr)
		enet_host_destroy(server);
}

int main(int argc, char** argv)
{
   signal(SIGINT, exit_handler);

   atexit(exitCleanup);

	registeredInstances = new ThorQ::InstanceMap();

	ENetAddress address;
	address.host = ENET_HOST_ANY;
#if PARSE_PORT
	if (argc < 2)
	{
		printf("Please provide port!\n");
		fflush(stdout);
		return EXIT_FAILURE;
	}
	else if (argc > 3)
	{
		printf("Too many arguments!\n");
		fflush(stdout);
		return EXIT_FAILURE;
	}

	try {
		int i = std::stoi(argv[1]);
		if (i < 1 || i > UINT16_MAX)
		{
			fprintf(stderr, "Port must be in the range of 1-65535\n");
			return EXIT_FAILURE;
		}
		address.port = i;
	} catch (std::invalid_argument ex) {
		fprintf(stderr, "Port must be a number\n");
		return EXIT_FAILURE;
	} catch (std::out_of_range) {
		fprintf(stderr, "Port must be in the range of 1-65535\n");
		return EXIT_FAILURE;
	} catch (std::exception ex) {
		fprintf(stderr, "Exception occured while parsing argument:\n\t%s\n", ex.what());
		return EXIT_FAILURE;
	} catch (int i) {
		fprintf(stderr, "Unknown exception occured while parsing argument\n");
		return EXIT_FAILURE;
	}
#else
	(void)argc;
	(void)argv;
	address.port = SERVER_PORT;
#endif
	if (enet_initialize() < 0)
	{
		printf("Failed to initialize ENet\n");
		return EXIT_FAILURE;
	}
	enet_was_initialized = true;

	printf("Using ENet-%i.%i.%i\n", ENET_VERSION_MAJOR, ENET_VERSION_MINOR, ENET_VERSION_PATCH);
	fflush(stdout);

	// Setup server
	server = enet_host_create(&address, SERVER_MAX_CONNECTIONS, 2, 0, 0); // two channels: communication(tcp), and commands(udp)

	if (server == nullptr)
	{
		printf("An error occurred while trying to create an ENet server host\n");
		return EXIT_FAILURE;
	}

	ENetEvent event;
	while (runServer.load()) {
		while (enet_host_service(server, &event, 0) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				handleEventNewConnection(event.peer);
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				handleEventMessage(event.peer, event.packet);
				enet_packet_destroy(event.packet);
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				handleEventDisconnect(event.peer);
				break;
			case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
				handleEventTimeout(event.peer);
				break;
			case ENET_EVENT_TYPE_NONE:
				break;
			}
		}
	}

	return EXIT_SUCCESS;
}
