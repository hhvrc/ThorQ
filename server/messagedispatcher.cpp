#include "messagedispatcher.h"

#include <enet.h>
#include <fmt/core.h>

#include <thorq_message.h>
#include <flatbuffers/flatbuffers.h>
#include <schemas/heartbeat_generated.h>
#include <schemas/version_generated.h>
#include <schemas/crypto_generated.h>
#include <schemas/systemid_generated.h>
#include <schemas/account_generated.h>
#include <schemas/session_generated.h>
#include <schemas/relationship_generated.h>
#include <schemas/moderation_generated.h>
#include <schemas/announcement_generated.h>
#include <schemas/collar_generated.h>

#include "utils.h"
#include "account.h"
#include "instance.h"
#include "messagehandlers.h"

ThorQ::MessageDispatcher::MessageDispatcher(ThorQ::Server *serverInstance)
    : m_buffer(THORQ_PAYLOAD_LEN_MAX)
{
}

void ThorQ::MessageDispatcher::DispatchEvent(const ENetEvent& event)
{
    ThorQ::Instance* instance = reinterpret_cast<ThorQ::Instance*>(event.peer->data);

    if (!instance->packetDecode(event.packet, m_buffer))
        return;
/*
    switch (event.channelID) {
    case THORQ_CHANNEL_MAIN:
    case THORQ_CHANNEL_IMPULSE:
    }

    switch (m_buffer[0]) {
    case THORQ_PAYLOAD_ID_HEARTBEAT:
        if (thorq_payload_heartbeat_is_valid(m_buffer))
        {
            handleMessageHeartbeat(instance, m_buffer);
            return;
        }
        break;
    case THORQ_PAYLOAD_ID_VERSION:
        if (thorq_payload_version_is_valid(m_buffer))
        {
            handleMessageVersion(instance, m_buffer);
            return;
        }
        break;
    case THORQ_PAYLOAD_ID_CRYPTO:
        if (thorq_payload_crypto_is_valid(m_buffer))
        {
            handleMessageCrypto(instance, m_buffer);
            return;
        }
        break;
    case THORQ_PAYLOAD_ID_SYSTEMID:
        if (thorq_payload_systemid_is_valid(m_buffer))
        {
            handleMessageSystemID(instance, m_buffer);
            return;
        }
        break;
    case THORQ_PAYLOAD_ID_ACCOUNT:
        if (thorq_payload_account_is_valid(m_buffer))
        {
            handleMessageAccount(instance, m_buffer);
            return;
        }
        break;
    case THORQ_PAYLOAD_ID_RELATIONSHIP:
        if (thorq_payload_relationship_is_valid(m_buffer))
        {
            handleMessageRelation(instance, m_buffer);
            return;
        }
        break;
    case THORQ_PAYLOAD_ID_SESSION:
        if (thorq_payload_session_is_valid(m_buffer))
        {
            handleMessageSession(instance, m_buffer);
            return;
        }
        break;
    case THORQ_PAYLOAD_ID_MODERATION:
        if (thorq_payload_moderation_is_valid(m_buffer))
        {
            handleMessageModeration(instance, m_buffer);
            return;
        }
        break;
    case THORQ_PAYLOAD_ID_COLLAR:
        if (thorq_payload_collar_is_valid(m_buffer))
        {
            handleMessageCollar(instance, m_buffer);
            return;
        }
        break;
    case THORQ_PAYLOAD_ID_ANNOUNCEMENT:
    case THORQ_PAYLOAD_ID_ACK:
        fmt::print("Unexpected messageID from client: {}\n", (int)payload[0]);
        fflush(stdout);
        break;
    default:
        if (instance->authState() != THORQ_STATE_AUTH_OK)
        {
            return;
        }

        break;
    }
    */
}
