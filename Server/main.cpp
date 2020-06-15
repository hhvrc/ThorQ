#include <iostream>

#define ENET_IMPLEMENTATION
#include "enet.h"

#include "enums.h"
#include "crypto.h"
#include "peermap.h"
#include "instance.h"

#include <botan/bcrypt.h>
#include <botan/system_rng.h>

#define SERVER_PORT 12345
#define SERVER_MAX_CONNECTIONS 1024

#define DISCONNECT_ERROR 0x00000001

using namespace ThorQ;

InstanceMap registeredInstances{};

std::string ExtractString(const std::uint8_t* data, std::size_t dataSize, std::size_t startOffset = 0, std::size_t endOffset = 0)
{
	return std::string(data + startOffset, data + dataSize - endOffset);
}

void BroadcastMessage(std::uint32_t meta, const std::string& message)
{
	std::size_t len = sizeof(std::uint32_t) + message.length();
	std::uint8_t* data = new std::uint8_t[len];
	memcpy(data, &meta, sizeof(std::uint32_t));
	memcpy(data + sizeof(std::uint32_t), message.data(), message.length());

	std::vector<Instance*> instances = registeredInstances.GetInstances();
	for (Instance* instance : instances)
		instance->SendEncrypted(data, len);
}

void handleMessage(ENetPeer* peer, ENetPacket* packet)
{
	Instance* instance = reinterpret_cast<Instance*>(peer->data);

	// Idk why this whould happen
	if (instance == nullptr)
	{
		enet_packet_destroy(packet);
		enet_peer_disconnect_now(peer, DISCONNECT_ERROR);
		return;
	}

	if (!instance->GetCrypto()->IsCryptoReady())
	{
		if (instance->GetCrypto()->Agree(packet->data, packet->dataLength))
		{
			std::cout << "Success!" << std::endl;
			return;
		}
		else
		{
			std::cout << "Failed to establish cryptographic link, disconnecting..." << std::endl;
			enet_packet_destroy(packet);
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

	instance->SetHasCollar((meta & FLAG_CollarConnected) != 0);

	meta &= 0xFF; // Remove flags from meta

	if (!instance->HasName())
	{
		if (meta == USER_Login)
		{
			std::string name = ExtractString(data.data(), data.size(), sizeof(std::uint32_t));

			if (!registeredInstances.TryAdd(name, instance))
			{
				instance->SendEncMessage(ACKNOWLEDGE_Denied, "Callname in use");
				return;
			}

			instance->SetName(name);
			instance->ClearPartner();

			BroadcastMessage(NOTIFY_UserOnline, name);
			return;
		}
		else
		{
			instance->SendEncMessage(ACKNOWLEDGE_Denied, "Please log in");
			return;
		}
	}

	switch (meta){
	case USER_Login:
	{
		instance->SendEncMessage(ACKNOWLEDGE_Denied, "Already logged in");
		break;
	}
	case USER_Logout:
	{
		// Remove from registered
		registeredInstances.Remove(instance->Name());

		// Disconnect session if one is ongoing
		if (instance->HasPartner())
		{
			instance->SendEncMessage(NOTIFY_SessionEnded, instance->Partner()->Name());
			instance->Partner()->SendEncMessage(NOTIFY_SessionEnded, instance->Name());

			BroadcastMessage(NOTIFY_UserAvailable, instance->Partner()->Name());

			instance->ClearPartner();
		}

		// Announce offline
		if (instance->HasName())
		{
			BroadcastMessage(NOTIFY_UserOffline, instance->Name());
			instance->SetName("");
		}
		instance->SendEncMessage(ACKNOWLEDGE_OK, "Logged out");
		break;
	}
	case USER_List:
	{
		std::vector<Instance*> instances = registeredInstances.GetInstances();

		for (Instance* i : instances)
		{
			std::uint32_t txFlag = NOTIFY_UserOnline;

			txFlag |= i->HasCollar() ? FLAG_CollarConnected : 0;

			instance->SendEncMessage(txFlag, i->Name());
		}
		break;
	}
	case SESSION_Request:
	{
		std::string name = ExtractString(data.data(), data.size(), sizeof(std::uint32_t));

		Instance* otherInstance = registeredInstances.GetInstance(name);

		if (otherInstance == nullptr)
		{
			instance->SendEncMessage(ACKNOWLEDGE_Denied, name + " is not online");
			return;
		}

		otherInstance->RequestOn(instance);
		break;
	}
	case SESSION_Accept:
	{
		std::string name = ExtractString(data.data(), data.size(), sizeof(std::uint32_t));

		Instance* otherInstance = registeredInstances.GetInstance(name);

		if (otherInstance == nullptr)
		{
			instance->SendEncMessage(ACKNOWLEDGE_Denied, name + " is not online");
			return;
		}

		instance->RequestAcceptFrom(otherInstance);

		BroadcastMessage(NOTIFY_UserInSession, instance->Name());
		BroadcastMessage(NOTIFY_UserInSession, otherInstance->Name());
		break;
	}
	case SESSION_Leave:
	{
		Instance* other = instance->Partner();
		instance->ClearPartner();

		if (other != nullptr)
		{
			BroadcastMessage(NOTIFY_UserInSession, instance->Name());
			BroadcastMessage(NOTIFY_UserInSession, other->Name());
		}
		break;
	}
	case COMMAND_Beep:
	case COMMAND_Vibrate:
	case COMMAND_Shock:
	case COMMAND_Auto:
	{
		if (instance->HasName() && instance->HasPartner())
			instance->Partner()->SendEncrypted(data, true);
		break;
	}
	case ACKNOWLEDGE_OK:
		std::cout << "Received ACK_OK" << std::endl;
		break;
	case ACKNOWLEDGE_Error:
		std::cout << "Received ACK_ERR" << std::endl;
		break;
	case ACKNOWLEDGE_Denied:
		std::cout << "Received ACK_DENIED" << std::endl;
		break;
	case ACKNOWLEDGE_Invalid:
		std::cout << "Received ACK_INVALID" << std::endl;
		break;
	default:
		std::cout << "Received unknown message id " << meta << std::endl;
		instance->SendEncMessage(ACKNOWLEDGE_Invalid, "Invalid message");
		break;
	}
}
std::string ipv6_to_str(const struct in6_addr& addr)
{
	char buffer[50];
	snprintf(buffer, sizeof(buffer), "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
				 (int)addr.s6_addr[0], (int)addr.s6_addr[1],
				 (int)addr.s6_addr[2], (int)addr.s6_addr[3],
				 (int)addr.s6_addr[4], (int)addr.s6_addr[5],
				 (int)addr.s6_addr[6], (int)addr.s6_addr[7],
				 (int)addr.s6_addr[8], (int)addr.s6_addr[9],
				 (int)addr.s6_addr[10], (int)addr.s6_addr[11],
				 (int)addr.s6_addr[12], (int)addr.s6_addr[13],
				 (int)addr.s6_addr[14], (int)addr.s6_addr[15]);
	return std::string(buffer);
}

void handleNewConnection(ENetPeer* peer)
{
	Instance* instance = new Instance(peer);
	printf("A new client connected from:\n\tIPV6: %llx\n\tPORT: %u\n", peer->address.host, peer->address.port);
	fflush(stdout);

	instance->SendRaw(instance->GetCrypto()->PublicKey());
}

void handleDisconnect(ENetPeer* peer)
{
	Instance* instance = reinterpret_cast<Instance*>(peer->data);

	if (instance == nullptr)
		return;

	if (!instance->HasName())
	{
		std::cout << "Unregistered client timed out" << std::endl;
		return;
	}
	else
	{
		std::cout << instance->Name() << " timed out" << std::endl;

		Instance* other = instance->Partner();
		instance->ClearPartner();

		if (other != nullptr)
		{
			BroadcastMessage(NOTIFY_UserInSession, instance->Name());
			BroadcastMessage(NOTIFY_UserInSession, other->Name());
		}

		registeredInstances.Remove(instance->Name());

		BroadcastMessage(NOTIFY_UserLostConnection, instance->Name());
	}

	delete instance;
	peer->data = nullptr;
}

void handleTimeout(ENetPeer* peer)
{
	Instance* instance = reinterpret_cast<Instance*>(peer->data);

	if (instance == nullptr)
		return;

	if (!instance->HasName())
	{
		std::cout << "Unregistered client timed out" << std::endl;
		return;
	}
	else
	{
		std::cout << instance->Name() << " timed out" << std::endl;

		Instance* other = instance->Partner();
		instance->ClearPartner();

		if (other != nullptr)
		{
			BroadcastMessage(NOTIFY_UserInSession, instance->Name());
			BroadcastMessage(NOTIFY_UserInSession, other->Name());
		}

		registeredInstances.Remove(instance->Name());

		BroadcastMessage(NOTIFY_UserTimedOut, instance->Name());
	}

	delete instance;
	peer->data = nullptr;
}

int main()
{
	if (enet_initialize() < 0)
		{
			printf("Failed to initialize ENet\n");
			exit(EXIT_FAILURE);
		}
		atexit(enet_deinitialize);

		printf("Using ENet-%i.%i.%i\n", ENET_VERSION_MAJOR, ENET_VERSION_MINOR, ENET_VERSION_PATCH);
		fflush(stdout);

		// Setup server
		ENetAddress address;
		address.host = ENET_HOST_ANY;
		address.port = SERVER_PORT;
		ENetHost* server = enet_host_create(&address, SERVER_MAX_CONNECTIONS, 2, 0, 0); // two channels: communication(tcp), and commands(udp)

		if (server == nullptr)
		{
			printf("An error occurred while trying to create an ENet server host\n");
			exit(EXIT_FAILURE);
		}

		ENetEvent event;
		while (true) {
			while (enet_host_service(server, &event, 0) > 0)
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
		}

		enet_host_destroy(server);

		exit(EXIT_SUCCESS);
}
