#include "messagedispatcher.h"

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

ThorQ::MessageDispatcher::MessageDispatcher(ThorQ::Server *serverInstance)
{

}

void ThorQ::MessageDispatcher::DispatchEvent(const ENetEvent& event)
{
    ThorQ::Instance* instance = reinterpret_cast<ThorQ::Instance*>(event.peer->data);

    if (!ThorQ::packetDecode(event.packet, m_buffer, instance->getCrypto()))
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
