#include "eventhandlers.h"

#include <enet.h>

#include "utils.h"
#include "instance.h"
#include "messagehandlers.h"

void handleEventNewConnection(ENetPeer* peer)
{
	// Dont worry, this is ok
	(void)new ThorQ::Instance(peer);

	printf("A new client connected from:\n\tIPV6: %s\n\tPORT: %u\n", enetaddr_to_str(&peer->address).c_str(), peer->address.port);
	fflush(stdout);
}

void handleEventMessage(ENetPeer* peer, ENetPacket* packet)
{
	printf("Got %lu bytes\n", packet->dataLength);
	fflush(stdout);

	ThorQ::Instance* instance = reinterpret_cast<ThorQ::Instance*>(peer->data);

	if (instance->connectionState() != THORQ_CONNECTION_STATE_CONNECTED || !thorq_message_is_valid(packet->data, packet->dataLength))
		return;

	std::vector<std::uint8_t> message;
	thorq_message_decode(packet->data, packet->dataLength, message, instance->getCrypto());

	printf("Payload is %lu bytes\n", message.size());
	fflush(stdout);

	thorq_payload_t payload;
	thorq_payload_unpack(message, payload);

	switch (payload.id) {
	case THORQ_PAYLOAD_ID_INVALID:
		printf("Got invalid payload!\n");
		fflush(stdout);
		break;
	case THORQ_PAYLOAD_ID_VERSION:
		if (thorq_payload_version_is_valid(payload))
		{
			handleMessageVersion(instance, payload);
			return;
		}
		break;
	case THORQ_PAYLOAD_ID_CRYPTO:
		if (thorq_payload_crypto_is_valid(payload))
		{
			handleMessageCrypto(instance, payload);
			return;
		}
		break;
	case THORQ_PAYLOAD_ID_AUTH:
		if (thorq_payload_auth_is_valid(payload))
		{
			handleMessageAuth(instance, payload);
			return;
		}
		break;
	case THORQ_PAYLOAD_ID_HEARTBEAT:
		if (thorq_payload_heartbeat_is_valid(payload))
		{
			handleMessageHeartbeat(instance, payload);
			return;
		}
		break;
	case THORQ_PAYLOAD_ID_COMMAND:
		if (thorq_payload_command_is_valid(payload))
		{
			handleMessageCommand(instance, payload);
			return;
		}
		break;
	case THORQ_PAYLOAD_ID_COMMAND_ACK:
		if (thorq_payload_command_ack_is_valid(payload))
		{
			handleMessageCommandAck(instance, payload);
			return;
		}
		break;
	case THORQ_PAYLOAD_ID_NOTIFICATION:
		if (thorq_payload_notification_is_valid(payload))
		{
			handleMessageNotification(instance, payload);
			return;
		}
		break;
	case THORQ_PAYLOAD_ID_COLLAR:
		if (thorq_payload_collar_is_valid(payload))
		{
			handleMessageCollar(instance, payload);
			return;
		}
		break;
	default:
		if (instance->authState() != THORQ_AUTH_STATE_OK)
		{
			return;
		}

		break;
	}

	if (!instance->hasName())
	{
		if (meta == USER_Login)
		{
			std::string name = ExtractString(data, size, sizeof(std::uint32_t));

			if (!registeredInstances.TryAdd(name, instance))
			{
				instance->setName(name);
				instance->clearPartner();

				thorq_payload_t txPayload;
				THORQ_PAYLOAD_ID_NOTIFICATION
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
		registeredInstances.Remove(instance->name());

		// Disconnect session if one is ongoing
		if (instance->hasPartner())
		{
			instance->SendEncrypted(NOTIFY_SessionEnded, instance->partner()->name());
			instance->partner()->SendEncrypted(NOTIFY_SessionEnded, instance->name());

			BroadcastMessage(NOTIFY_UserAvailable, instance->partner()->name());

			instance->clearPartner();
		}

		// Announce offline
		if (instance->hasName())
		{
			BroadcastMessage(NOTIFY_UserOffline, instance->name());
			instance->setName("");
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

			txFlag |= i->hasCollar() ? FLAG_CollarConnected : 0;

			instance->SendEncrypted(txFlag, i->name());
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

		otherInstance->requestOn(instance);
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

		instance->requestAcceptFrom(otherInstance);

		BroadcastMessage(NOTIFY_UserInSession, instance->name());
		BroadcastMessage(NOTIFY_UserInSession, otherInstance->name());
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

		if (instance->requestAcceptFrom(otherInstance))
		{
			BroadcastMessage(NOTIFY_UserInSession, instance->name());
			BroadcastMessage(NOTIFY_UserInSession, otherInstance->name());
		}
		break;
	}
	case SESSION_Leave:
	{
		Instance* other = instance->partner();
		instance->clearPartner();

		if (other != nullptr)
		{
			BroadcastMessage(NOTIFY_UserInSession, instance->name());
			BroadcastMessage(NOTIFY_UserInSession, other->name());
		}
		break;
	}
	case COMMAND_Beep:
	case COMMAND_Vibrate:
	case COMMAND_Shock:
	{
		if (instance->hasName() && instance->hasPartner())
			instance->partner()->SendEncrypted(data, true);
		break;
	}
	case COMMAND_Auto:
	{
		if (instance->hasName() && instance->hasPartner())
			instance->partner()->SendEncrypted(data, false);
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

void handleEventDisconnect(ENetPeer* peer)
{
	Instance* instance = reinterpret_cast<Instance*>(peer->data);

	if (instance == nullptr)
		return;

	if (!instance->hasName())
	{
		std::cout << "Unregistered client timed out" << std::endl;
		return;
	}
	else
	{
		std::cout << instance->name() << " timed out" << std::endl;

		Instance* other = instance->partner();
		instance->clearPartner();

		if (other != nullptr)
		{
			BroadcastMessage(NOTIFY_UserInSession, instance->name());
			BroadcastMessage(NOTIFY_UserInSession, other->name());
		}

		registeredInstances.Remove(instance->name());

		BroadcastMessage(NOTIFY_UserLostConnection, instance->name());
	}

	peer->data = nullptr;
	delete instance;
}

void handleEventTimeout(ENetPeer* peer)
{
	Instance* instance = reinterpret_cast<Instance*>(peer->data);

	if (instance == nullptr)
		return;

	if (!instance->hasName())
	{
		std::cout << "Unregistered client timed out" << std::endl;
		return;
	}
	else
	{
		std::cout << instance->name() << " timed out" << std::endl;

		Instance* other = instance->partner();
		instance->clearPartner();

		if (other != nullptr)
		{
			BroadcastMessage(NOTIFY_UserInSession, instance->name());
			BroadcastMessage(NOTIFY_UserInSession, other->name());
		}

		registeredInstances.Remove(instance->name());

		BroadcastMessage(NOTIFY_UserTimedOut, instance->name());
	}

	delete instance;
	peer->data = nullptr;
}
