#include "eventhandlers.h"

#include <QDebug>
#include <QString>

#include <enet.h>

#include <thorq_message.h>
#include <thorq_payload_heartbeat.h>
#include <thorq_payload_version.h>
#include <thorq_payload_crypto.h>
#include <thorq_payload_systemid.h>
#include <thorq_payload_account.h>
#include <thorq_payload_session.h>
//#include <thorq_payload_friend.h>
//#include <thorq_payload_room.h>
//#include <thorq_payload_moderation.h>
#include <thorq_payload_announcement.h>
#include <thorq_payload_collar.h>
#include <thorq_payload_ack.h>

#include "utils.h"
#include "account.h"
#include "instance.h"
#include "messagehandlers.h"

void handleEventNewConnection(ENetPeer* peer)
{
    // Dont worry, its ok to have a seemingly dangling pointer here (ENet keeps track of the pointer)

    ThorQ::Instance* instance = new ThorQ::Instance(peer);

    std::vector<std::uint8_t> message;

    thorq_payload_version_pack(message, THORQ_APP_LINK, THORQ_VERSION_LINK);
    instance->sendMessage(message, false, true);

    thorq_payload_version_pack(message, THORQ_APP_CLIENT, THORQ_VERSION_CLIENT);
    instance->sendMessage(message, false, true);

    thorq_payload_version_pack(message, THORQ_APP_SERVER, THORQ_VERSION_SERVER);
    instance->sendMessage(message, false, true);

	thorq_payload_heartbeat_pack(message, 500); // TODO: get from config
	instance->sendMessage(message, false, true);

    qDebug() << QString("A new client connected from:\n\tIPV6: %1\n\tPORT: %2")
                .arg(enet_peer_address_str(peer))
                .arg(peer->address.port);
}

void handleEventMessage(ENetPeer* peer, ENetPacket* packet)
{
	if (peer->data == nullptr)
		return;

    auto instance = reinterpret_cast<ThorQ::Instance*>(peer->data);

	std::vector<std::uint8_t> message(packet->data, packet->data + packet->dataLength);

    if (!thorq_message_decode(message, instance->getCrypto()))
        return;

    switch (message[0]) {
    case THORQ_PAYLOAD_ID_HEARTBEAT:
        if (thorq_payload_heartbeat_is_valid(message))
        {
            handleMessageHeartbeat(instance, message);
            return;
        }
        break;
    case THORQ_PAYLOAD_ID_VERSION:
        if (thorq_payload_version_is_valid(message))
        {
            handleMessageVersion(instance, message);
            return;
        }
        break;
    case THORQ_PAYLOAD_ID_CRYPTO:
        if (thorq_payload_crypto_is_valid(message))
        {
            handleMessageCrypto(instance, message);
            return;
        }
        break;
    case THORQ_PAYLOAD_ID_SYSTEMID:
        if (thorq_payload_systemid_is_valid(message))
        {
            handleMessageSystemID(instance, message);
            return;
        }
        break;
    case THORQ_PAYLOAD_ID_ACCOUNT:
        if (thorq_payload_account_is_valid(message))
        {
            handleMessageAccount(instance, message);
            return;
        }
        break;
    case THORQ_PAYLOAD_ID_RELATION:
        if (thorq_payload_relation_is_valid(message))
        {
            handleMessageRelation(instance, message);
            return;
        }
        break;
    case THORQ_PAYLOAD_ID_SESSION:
        if (thorq_payload_session_is_valid(message))
        {
            handleMessageSession(instance, message);
            return;
        }
        break;
    case THORQ_PAYLOAD_ID_MODERATION:
        if (thorq_payload_moderation_is_valid(message))
		{
            handleMessageModeration(instance, message);
			return;
		}
        break;
    case THORQ_PAYLOAD_ID_COLLAR:
        if (thorq_payload_collar_is_valid(message))
        {
            handleMessageCollar(instance, message);
            return;
        }
        break;
    case THORQ_PAYLOAD_ID_ANNOUNCEMENT:
    case THORQ_PAYLOAD_ID_ACK:
        qDebug() << "Waitttttt... im not supposed to get these?" << (int)message[0];
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

    if (instance->account() != nullptr)
    {
        qDebug() << "User" << instance->account()->username()
                 << "connected from [" << enet_peer_address_str(peer) << "] disconnected";
    }
    else
    {
        qDebug() << "User connected from [" << enet_peer_address_str(peer) << "] disconnected";
    }

	peer->data = nullptr;

    // Automatically notifies others
    instance->deleteLater();
}

void handleEventTimeout(ENetPeer* peer)
{
	if (peer->data == nullptr)
		return;

    auto instance = reinterpret_cast<ThorQ::Instance*>(peer->data);

    if (instance->account() != nullptr)
    {
        qDebug() << "User" << instance->account()->username()
                 << "connected from [" << enet_peer_address_str(peer) << "] timed out";
    }
    else
    {
        qDebug() << "User connected from [" << enet_peer_address_str(peer) << "] timed out";
    }

    peer->data = nullptr;

    // Automatically notifies others
    instance->deleteLater();
}
