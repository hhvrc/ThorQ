#include "eventhandlers.h"

#include <log.h>
#include <enet.h>

#include <thorq_message.h>
#include <thorq_payload.h>
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

	thorq_payload_t payload;

	thorq_payload_version_pack(payload, THORQ_APP_LINK,   THORQ_VERSION_LINK);
	instance->sendPayload(&payload, false, true);

	thorq_payload_version_pack(payload, THORQ_APP_CLIENT, THORQ_VERSION_CLIENT);
	instance->sendPayload(&payload, false, true);

	thorq_payload_version_pack(payload, THORQ_APP_SERVER, THORQ_VERSION_SERVER);
	instance->sendPayload(&payload, false, true);

	thorq_debug_fmt("A new client connected from:\n\tIPV6: %s\n\tPORT: %u\n", enetaddr_to_str(&peer->address).c_str(), peer->address.port)
	fflush(stdout);
}

void handleEventMessage(ENetPeer* peer, ENetPacket* packet)
{
	if (peer->data == nullptr)
		return;

	auto instance = reinterpret_cast<ThorQ::Instance*>(peer->data);

	if (instance->connectionState() != THORQ_STATE_CONNECTION_CONNECTED || !thorq_message_is_valid(packet->data, packet->dataLength))
		return;

	std::vector<std::uint8_t> message;
	thorq_message_decode(packet->data, packet->dataLength, message, instance->getCrypto());

	thorq_payload_t payload;
	thorq_payload_unpack(message, payload);

	switch (payload.id) {
	case THORQ_PAYLOAD_ID_COLLAR:
		if (thorq_payload_collar_is_valid(payload))
		{
			handleMessageCollar(instance, message);
			return;
		}
		break;
	case THORQ_PAYLOAD_ID_HEARTBEAT:
		if (thorq_payload_heartbeat_is_valid(payload))
		{
			handleMessageHeartbeat(instance);
			return;
		}
		break;
	case THORQ_PAYLOAD_ID_VERSION:
		if (thorq_payload_version_is_valid(payload))
		{
			handleMessageVersion(instance, &payload);
			return;
		}
		break;
	case THORQ_PAYLOAD_ID_CRYPTO:
		if (thorq_payload_crypto_is_valid(payload))
		{
			handleMessageCrypto(instance, &payload);
			return;
		}
		break;
	case THORQ_PAYLOAD_ID_AUTH:
		if (thorq_payload_auth_is_valid(payload))
		{
			handleMessageAuth(instance, &payload);
			return;
		}
		break;
	case THORQ_PAYLOAD_ID_COMMAND:
		if (thorq_payload_command_is_valid(payload))
		{
			handleMessageCommand(instance, &payload);
			return;
		}
		break;
	case THORQ_PAYLOAD_ID_COMMAND_ACK:
		if (thorq_payload_command_ack_is_valid(payload))
		{
			handleMessageCommandAck(instance, &payload);
			return;
		}
        break;
    case THORQ_PAYLOAD_ID_EVENT:
    case THORQ_PAYLOAD_ID_ANNOUNCEMENT:
	case THORQ_PAYLOAD_ID_INVALID:
        thorq_debug_fmt("Got invalid payload: %i\n", payload.id);
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
        thorq_payload_t payload;
        thorq_payload_notification_pack(payload, THORQ_NOTIFICATION_USER_OFFLINE_TIMEOUT, instance->name());
        instance->sendPayload(&payload, true, true);

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
