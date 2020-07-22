#include <iostream>
#include <exception>

#define ENET_IMPLEMENTATION
#include <enet.h>

#include <enums.h>
#include <crypto.h>
#include <thorq_message_announcement.h>

#include "peermap.h"
#include "instance.h"

#define PARSE_PORT false
#define SERVER_PORT 12345
#define SERVER_MAX_CONNECTIONS 1024

#define DISCONNECT_ERROR 0x00000001

using namespace ThorQ;

ENetHost* server;
InstanceMap registeredInstances{};

void BroadcastMessage(const thorq_announcement_t& message, bool reliable = true)
{


	enet_host_broadcast(server, 0, enet_packet_create());
}
void BroadcastMessage(const thorq_message_t& message, bool reliable = true)
{
	std::vector<Instance*> instances = registeredInstances.GetInstances();

	for (Instance* instance : instances)
		SendMsg(instance->Peer(), message, instance->GetCrypto(), reliable);
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

	if (instance->ConnectionState() != THORQ_CONNECTION_STATE_CONNECTED || packet->dataLength < sizeof(std::uint8_t))
		return;

	Message msg = Message::Deserialize(packet->data, packet->dataLength);

	printf("Got %lu bytes\n", msg.payloadSize());
	fflush(stdout);

	if (msg.IsHeartbeat())
	{
		Message::NewHeartbeat().Send(instance->Peer(), false);
		printf("Heartbeat\n");
		fflush(stdout);
		return;
	}

	switch (msg.Meta()) {
	case ThorQ::MessageContentEnums::CRYPT_REQUEST:
	{
		instance->CryptoInit();
		printf("Got request\n");
		fflush(stdout);
		return;
	}
	case ThorQ::MessageContentEnums::CRYPT_ESTABLISH:
	{
		printf("Got establish\n");
		fflush(stdout);
		if (instance->CryptoEstablish(msg.payload()))
		{
			printf("Establish complete\n");
			fflush(stdout);
		}
		return;
	}
	case ThorQ::MessageContentEnums::CRYPT_VERIFY:
	{
		if (msg.payloadSize() != 0 && msg.IsEncrypted() && instance->GetCrypto()->ready())
		{
			msg.Decrypt(instance->GetCrypto());
			printf("Got verify\n");
			fflush(stdout);
			if (instance->CryptoVerify(msg.payload()))
			{
				printf("HANDSHAKE COMPLETE\n");
				fflush(stdout);
			}
		}
		return;
	}
	case ThorQ::MessageContentEnums::CRYPT_OK:
	{
		if (data == nullptr || size == 0)
			return;
		break;
	}
	default:
	{
		printf("Got unknown HeaderEnum: %i\n", flag);
		fflush(stdout);
		instance->SetCryptoState(ThorQ::CryptoState::None);
		instance->SetClientState(ThorQ::ClientState::Connecting);
		return;
	}
	}

	if (!instance->GetCrypto()->ready())
        return;

	std::vector<std::uint8_t> vec = instance->GetCrypto()->Decrypt(data, size);

	std::uint8_t meta = static_cast<std::uint8_t>(*vec.data());

	instance->SetHasCollar((meta & FLAG_CollarConnected) != 0);

	if (!instance->HasName())
	{
		if (meta == USER_Login)
		{
			std::string name = ExtractString(data, size, sizeof(std::uint32_t));

			if (!registeredInstances.TryAdd(name, instance))
			{
				instance->SetName(name);
				instance->ClearPartner();

				BroadcastMessage(NOTIFY_UserOnline, name);
				instance->SendEncrypted(ACKNOWLEDGE_OK, "Logged in");
			}
			else
			{
				instance->SendEncrypted(ACKNOWLEDGE_Denied, "Callname in use");
			}
		}
		else
		{
			instance->SendEncrypted(ACKNOWLEDGE_Denied, "Please log in");
		}
		return;
	}

	switch (meta){
	case USER_Login:
	{
		instance->SendEncrypted(ACKNOWLEDGE_Denied, "Already logged in");
		break;
	}
	case USER_Logout:
	{
		// Remove from registered
		registeredInstances.Remove(instance->Name());

		// Disconnect session if one is ongoing
		if (instance->HasPartner())
		{
			instance->SendEncrypted(NOTIFY_SessionEnded, instance->Partner()->Name());
			instance->Partner()->SendEncrypted(NOTIFY_SessionEnded, instance->Name());

			BroadcastMessage(NOTIFY_UserAvailable, instance->Partner()->Name());

			instance->ClearPartner();
		}

		// Announce offline
		if (instance->HasName())
		{
			BroadcastMessage(NOTIFY_UserOffline, instance->Name());
			instance->SetName("");
		}
		instance->SendEncrypted(ACKNOWLEDGE_OK, "Logged out");
		break;
	}
	case USER_List:
	{
		std::vector<Instance*> instances = registeredInstances.GetInstances();

		for (Instance* i : instances)
		{
			std::uint32_t txFlag = NOTIFY_UserOnline;

			txFlag |= i->HasCollar() ? FLAG_CollarConnected : 0;

			instance->SendEncrypted(txFlag, i->Name());
		}
		break;
	}
	case SESSION_Request:
	{
		std::string name = ExtractString(data, size, sizeof(std::uint32_t));

		Instance* otherInstance = registeredInstances.GetInstance(name);

		if (otherInstance == nullptr)
		{
			instance->SendEncrypted(ACKNOWLEDGE_Denied, name + " is not online");
			return;
		}

		otherInstance->RequestOn(instance);
		break;
	}
	case SESSION_Accept:
	{
		std::string name = ExtractString(data, size, sizeof(std::uint32_t));

		Instance* otherInstance = registeredInstances.GetInstance(name);

		if (otherInstance == nullptr)
		{
			instance->SendEncrypted(ACKNOWLEDGE_Denied, name + " is not online");
			return;
		}

		instance->RequestAcceptFrom(otherInstance);

		BroadcastMessage(NOTIFY_UserInSession, instance->Name());
		BroadcastMessage(NOTIFY_UserInSession, otherInstance->Name());
		break;
	}
	case SESSION_Deny:
	{
		std::string name = ExtractString(data, size, sizeof(std::uint32_t));

		Instance* otherInstance = registeredInstances.GetInstance(name);

		if (otherInstance == nullptr)
		{
			instance->SendEncrypted(ACKNOWLEDGE_Denied, name + " is not online");
			return;
		}

		if (instance->RequestAcceptFrom(otherInstance))
		{
			BroadcastMessage(NOTIFY_UserInSession, instance->Name());
			BroadcastMessage(NOTIFY_UserInSession, otherInstance->Name());
		}
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
	{
		if (instance->HasName() && instance->HasPartner())
			instance->Partner()->SendEncrypted(data, true);
		break;
	}
	case COMMAND_Auto:
	{
		if (instance->HasName() && instance->HasPartner())
			instance->Partner()->SendEncrypted(data, false);
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
		instance->SendEncrypted(ACKNOWLEDGE_Invalid, "Invalid message");
		break;
	}
}
std::string enetaddr_to_str(const ENetAddress* addr)
{
	char buffer[50];
	if (enet_address_get_host_ip(addr, buffer, 50) < 0)
		return "ERROR";
	return std::string(buffer);
}

void handleNewConnection(ENetPeer* peer)
{
	// Dont worry, this is ok
	Instance* instance = new Instance(peer);

	printf("A new client connected from:\n\tIPV6: %s\n\tPORT: %u\n", enetaddr_to_str(&peer->address).c_str(), peer->address.port);
	fflush(stdout);
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

int main(int argc, char** argv)
{
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
	address.port = SERVER_PORT;
#endif
	if (enet_initialize() < 0)
	{
		printf("Failed to initialize ENet\n");
		return EXIT_FAILURE;
	}
	atexit(enet_deinitialize);

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

	return EXIT_SUCCESS;
}
