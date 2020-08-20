#include "eventhandlers.h"

#include <log.h>
#include <enet.h>

#include <thorq_message.h>
#include <thorq_payload_version.h>
#include <thorq_payload_heartbeat.h>
#include <thorq_payload_crypto.h>
#include <thorq_payload_auth.h>
#include <thorq_payload_command.h>
#include <thorq_payload_command_ack.h>
#include <thorq_payload_event.h>
#include <thorq_payload_announcement.h>
#include <thorq_payload_notification.h>
#include <thorq_payload_collar.h>

#include "utils.h"
#include "instance.h"
#include "instancemap.h"
#include "messagehandlers.h"

void handleEventNewConnection(ENetPeer* peer)
{
	// Dont worry, this is ok
	ThorQ::Instance* instance = new ThorQ::Instance(peer);
	instance->setConnectionState(THORQ_STATE_CONNECTION_CONNECTED);

    std::vector<std::uint8_t> message;

    thorq_message_version_pack(message, THORQ_APP_LINK,   THORQ_VERSION_LINK);
    instance->sendMessage(message, false, true);

    thorq_message_version_pack(message, THORQ_APP_CLIENT, THORQ_VERSION_CLIENT);
    instance->sendMessage(message, false, true);

    thorq_message_version_pack(message, THORQ_APP_SERVER, THORQ_VERSION_SERVER);
    instance->sendMessage(message, false, true);

	thorq_debug_fmt("A new client connected from:\n\tIPV6: %s\n\tPORT: %u\n", enetaddr_to_str(&peer->address).c_str(), peer->address.port)
	fflush(stdout);
}

void handleEventMessage(ENetPeer* peer, ENetPacket* packet)
{
	if (peer->data == nullptr)
		return;

	auto instance = reinterpret_cast<ThorQ::Instance*>(peer->data);

	if (instance->connectionState() != THORQ_STATE_CONNECTION_CONNECTED)
		return;

	std::vector<std::uint8_t> message(packet->data, packet->data + packet->dataLength);

	if (!thorq_message_decode(message, instance->getCrypto()))
        return;

    switch (message[0]) {
	case THORQ_MESSAGE_ID_COLLAR:
        if (thorq_message_collar_is_valid(message))
		{
			handleMessageCollar(instance, message);
			return;
		}
		break;
	case THORQ_MESSAGE_ID_HEARTBEAT:
        if (thorq_message_heartbeat_is_valid(message))
		{
			handleMessageHeartbeat(instance);
			return;
		}
		break;
	case THORQ_MESSAGE_ID_VERSION:
        if (thorq_message_version_is_valid(message))
		{
            handleMessageVersion(instance, message);
			return;
		}
		break;
	case THORQ_MESSAGE_ID_CRYPTO:
        if (thorq_message_crypto_is_valid(message))
		{
            handleMessageCrypto(instance, message);
			return;
		}
		break;
	case THORQ_MESSAGE_ID_AUTH:
        if (thorq_message_auth_is_valid(message))
		{
            handleMessageAuth(instance, message);
			return;
		}
		break;
	case THORQ_MESSAGE_ID_COMMAND:
        if (thorq_message_command_is_valid(message))
		{
            handleMessageCommand(instance, message);
			return;
		}
		break;
	case THORQ_MESSAGE_ID_COMMAND_ACK:
        if (thorq_message_command_ack_is_valid(message))
		{
            handleMessageCommandAck(instance, message);
			return;
		}
        break;
    case THORQ_MESSAGE_ID_EVENT:
    case THORQ_MESSAGE_ID_ANNOUNCEMENT:
	case THORQ_MESSAGE_ID_INVALID:
        thorq_debug_fmt("Got invalid message: %u\n", message[0]);
		fflush(stdout);
        break;
	default:
		if (instance->authState() != THORQ_STATE_AUTH_OK)
		{
			return;
		}

		break;
	}
}

void handleEventDisconnect(ENetPeer* peer)
{
	if (peer->data == nullptr)
		return;

    auto instance = reinterpret_cast<ThorQ::Instance*>(peer->data);

    if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
    {
        thorq_debug_fmt("User \"%s\" connected from [%s] disconnected", instance->name().c_str(), enetaddr_to_str(&peer->address).c_str());
    }
    else
    {
        thorq_debug_fmt("Client connected from [%s] disconnected", enetaddr_to_str(&peer->address).c_str());
    }
    fflush(stdout);

    // Automatically notifies and handles disconnection
    instance->setConnectionState(THORQ_STATE_CONNECTION_DISCONNECTED);

	peer->data = nullptr;
	delete instance;
}

void handleEventTimeout(ENetPeer* peer)
{
	if (peer->data == nullptr)
		return;

    auto instance = reinterpret_cast<ThorQ::Instance*>(peer->data);

    if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
    {
        std::vector<std::uint8_t> message;
        thorq_message_notification_pack(message, THORQ_NOTIFICATION_USER_OFFLINE_TIMEOUT, instance->name());
        instance->sendMessage(message, true, true);

        thorq_debug_fmt("User \"%s\" connected from [%s] timed out", instance->name().c_str(), enetaddr_to_str(&peer->address).c_str());
    }
    else
    {
        thorq_debug_fmt("Client connected from [%s] timed out", enetaddr_to_str(&peer->address).c_str());
    }
	fflush(stdout);

    // Automatically notifies and handles disconnection
    instance->setConnectionState(THORQ_STATE_CONNECTION_DISCONNECTED);

	peer->data = nullptr;
	delete instance;
}
