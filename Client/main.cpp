#include <iostream>

#define ENET_IMPLEMENTATION
#include <enet.h>

#include <enums.h>
#include <crypto.h>

#define SERVER_HOSTNAME "::1"
#define SERVER_PORT     12345

#define DISCONNECT_ERROR 0x00000001

#include "instance.h"

#include <chrono>
ThorQ::Instance* currentInstance = nullptr;

std::chrono::high_resolution_clock::time_point pingSentTime;

std::string ExtractString(const std::uint8_t* data, std::size_t dataSize, std::size_t startOffset = 0, std::size_t endOffset = 0)
{
	return std::string(data + startOffset, data + dataSize - endOffset);
}

void handleMessage(ENetPeer* peer, ENetPacket* packet)
{
	ThorQ::Instance* instance = (ThorQ::Instance*)peer->data;

	if (!instance->GetCrypto()->IsCryptoReady())
	{
		if (instance->GetCrypto()->Agree(packet->data, packet->dataLength))
		{
			std::vector<std::uint8_t> data = instance->GetCrypto()->PublicKey();

			enet_peer_send(peer, 0, enet_packet_create(data.data(), data.size(), ENET_PACKET_FLAG_RELIABLE));
			return;
		}
		else
		{
			std::cout << "Failed to establish cryptographic link, disconnecting..." << std::endl;
			enet_peer_disconnect_now(peer, DISCONNECT_ERROR);
			return;
		}
	}

	std::vector<std::uint8_t> data = instance->GetCrypto()->Decrypt(packet->data, packet->dataLength);

	if (data.empty())
	{
		std::cout << "Data is empty!" << std::endl;
		return;
	}

	std::uint32_t meta = ntohl(*(std::int32_t*)data.data());

	if ((meta & 0xFF) == ThorQ::ThorqEnums::HEARTBEAT)
	{
		std::cout << "Latency: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - pingSentTime).count() / 100.f << "ms" << std::endl;
	}
}

void handleNewConnection(ENetPeer* peer)
{
	printf("Connected to server:\n\tIPV6: %llx\n\tPORT: %u\n", peer->address.host, peer->address.port);
	fflush(stdout);

	currentInstance = new ThorQ::Instance(peer);
}

void handleDisconnect(ENetPeer* peer)
{
	printf("%s disconnected\n", peer->data);
	fflush(stdout);
	peer->data = nullptr;
}

void handleTimeout(ENetPeer* peer)
{
	printf("%s timed out\n", peer->data);
	fflush(stdout);
	peer->data = nullptr;
}

#include "gui.h"

int main()
{
	// TODO: make GUI the main thread, and Networking a seperate thread
	// TODO: customize GUI
	// TODO: enable support for SteamVR
	// TODO: Add pre-encryption flag that signalises if connection is encrypted or not so clients can re-authenticate

	Gui* gui = Gui::CreateGui("woooooooo", 1000, 500);
	gui->Run();
	delete gui;

	if (enet_initialize() < 0)
	{
		printf("Failed to initialize ENet");
		exit(EXIT_FAILURE);
	}
	atexit(enet_deinitialize);

	printf("Using ENet-%i.%i.%i\n", ENET_VERSION_MAJOR, ENET_VERSION_MINOR, ENET_VERSION_PATCH);
	fflush(stdout);

	ENetEvent event;
	ENetPeer* peer;
	ENetHost* client;

	// Setup client
	client = enet_host_create(nullptr, 1, 2, 0, 0);

	// Connect client
	ENetAddress address;
	enet_address_set_host(&address, SERVER_HOSTNAME);
	address.port = SERVER_PORT;
	peer = enet_host_connect(client, &address, 4, 0);

	if (peer == nullptr)
	{
		fprintf(stderr, "No available peers for initiating an ENet connection.\n");
		exit(EXIT_FAILURE);
	}

	while (true) {
		while (enet_host_service(client, &event, 500) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				handleNewConnection(event.peer);
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				handleMessage(event.peer, event.packet);
				enet_packet_destroy(event.packet);
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				handleDisconnect(event.peer);
				break;
			case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
				handleTimeout(event.peer);
				break;
			case ENET_EVENT_TYPE_NONE:
				break;
			}
		}

		pingSentTime = std::chrono::high_resolution_clock::now();
		currentInstance->SendEncMessage(ThorQ::ThorqEnums::HEARTBEAT);
	}

	enet_host_destroy(client);

	exit(EXIT_SUCCESS);
}
