#include "endpoint_friendrequest.h"

#include "apiserver_connection.h"
#include "messagehandlingcontext.h"

#include <schemas_common.h>
#include <fmt/core.h>

void ThorQ::ApiEndpoints::FriendRequestEndpoint::handleMessage(ThorQ::HandlerContext& context)
{
    auto msgBody = context.body<ThorQ::Serialization::FriendRequest::Message>();
    auto subBody = msgBody->body();
    auto connection = context.apiConnection();

    if (subBody == nullptr) {
        connection->disconnect();
        return;
    }

    fmt::print("[MSG] FriendRequest\n");

    switch (msgBody->body_type()) {
    case ThorQ::Serialization::FriendRequest::Body_request:
    case ThorQ::Serialization::FriendRequest::Body_incoming:
    case ThorQ::Serialization::FriendRequest::Body_incoming_accept:
    case ThorQ::Serialization::FriendRequest::Body_incoming_deny:
    case ThorQ::Serialization::FriendRequest::Body_outgoing:
    case ThorQ::Serialization::FriendRequest::Body_outgoing_remove:
    case ThorQ::Serialization::FriendRequest::Body_accepted:
    case ThorQ::Serialization::FriendRequest::Body_removed:
    default:
        fmt::print("[MSG][FRIENDREQUEST] Invalid\n");
        // TODO: THORQ_DISCONNECT_REASON::INVALID_REQUEST
        connection->disconnect();
        return;
    }

    /*
    std::vector<std::uint8_t> response;

    THORQ_COMMAND_ID cmd;
    thorq_payload_command_get_id(message, cmd);

    if (instance->authState() != THORQ_STATE_AUTH_OK)
    {
        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_COMMAND, cmd, THORQ_PAYLOAD_ACK_UNAUTHORIZED);
        instance->packetSend(response, false, true);
        return;
    }

    switch (cmd){
    case THORQ_COMMAND_ID_GET_USER_LIST:
    {
        if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_COMMAND, cmd, THORQ_PAYLOAD_ACK_OK);
            instance->packetSend(response, true, true);

            std::vector<ThorQ::Instance*> instances = g_sessions.toList();

            for (ThorQ::Instance* i : instances)
            {
                if (i->account() != nullptr)
                {
                    thorq_payload_notification_pack(response, THORQ_NOTIFICATION_USER_ACTIVITY, i->account()->username(), i->activityState());
                    instance->packetSend(response, true, true);
                }
            }
        }
        else
        {
            thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_COMMAND, cmd, THORQ_PAYLOAD_ACK_LOGIN_NEEDED);
            instance->packetSend(response, true, true);
        }
        break;
    }
    case THORQ_COMMAND_ID_SESSION_REQUEST:
    {
        if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            std::string username;
            thorq_payload_command_get_data(message, username);

            auto it = std::find_if(g_accounts.begin(), g_accounts.end(), [&](const std::shared_ptr<ThorQ::Account> account) -> bool
            {
                return account->username() == username;
            });

            if (*it == nullptr)
            {
                thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_SESSION, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_PAYLOAD_ACK_DENIED, username + "is not an account");
                instance->packetSend(response, true);
                return;
            }

            std::unordered_setThorQ::Instance*> targetInstances = (*it)->instances();

            if (targetInstances.isEmpty())
            {
                thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_SESSION, THORQ_PAYLOAD_ACK_DENIED, username + "is not online");
                instance->packetSend(response, true);
                return;
            }

            for (ThorQ::Instance* otherInstance : (*it)->instances())
            instance->requestOn(otherInstance);
        }
        else
        {
            thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_COMMAND, cmd, THORQ_PAYLOAD_ACK_LOGIN_NEEDED);
            instance->packetSend(response, true, true);
        }
        break;
    }
    case THORQ_COMMAND_ID_SESSION_ACCEPT:
    {
        if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            std::string name;
            thorq_payload_command_get_data(message, name);

            ThorQ::Instance* otherInstance = g_sessions->get(name);

            if (otherInstance == nullptr)
            {
                thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_COMMAND, cmd, THORQ_PAYLOAD_ACK_DENIED, name + "is not online");
                instance->packetSend(response, true);
                return;
            }

            instance->requestAcceptFrom(otherInstance);
        }
        else
        {
            thorq_payload_ack_pack(response, cmd, THORQ_PAYLOAD_ACK_LOGIN_NEEDED);
            instance->packetSend(response, true, true);
        }
        break;
    }
    case THORQ_PAYLOAD_ROOM_
    {
        if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            std::string name;
            thorq_payload_
            thorq_payload_command_get_data(message, name);

            auto sit = std::find_if(g_accounts.begin(), g_accounts.end(), [name](const std::shared_ptr<ThorQ::Account> a) -> bool
            {
                if (a == nullptr) return false;

                return a->username() == name;
            });
            ThorQ::Instance* otherInstance = g_sessions .get(name);

            if (otherInstance == nullptr)
            {
                thorq_payload_ack_pack(response, cmd, THORQ_PAYLOAD_ACK_DENIED, name + "is not online");
                instance->packetSend(response, true);
                return;
            }

            instance->requestDenyFrom(otherInstance);
        }
        else
        {
            thorq_payload_ack_pack(response, cmd, THORQ_PAYLOAD_ACK_LOGIN_NEEDED);
            instance->packetSend(response, true, true);
        }
        break;
    }
    case THORQ_COMMAND_ID_SESSION_LEAVE:
    {
        if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            instance->setSessionState(THORQ_STATE_SESSION_NONE);

            thorq_payload_ack_pack(response, cmd, THORQ_PAYLOAD_ACK_OK);
            instance->packetSend(response, true, true);
        }
        else
        {
            thorq_payload_ack_pack(response, cmd, THORQ_PAYLOAD_ACK_LOGIN_NEEDED);
            instance->packetSend(response, true, true);
        }
        break;
    }
    case THORQ_COMMAND_ID_SET_SELF_STATE:
    {
        if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            std::uint8_t state;
            thorq_payload_command_get_data(message, state);
            instance->setActivityState(state);
        }
        else
        {
            thorq_payload_ack_pack(response, cmd, THORQ_PAYLOAD_ACK_LOGIN_NEEDED);
            instance->packetSend(response, true, true);
        }
        break;
    }
    }
    */
}

void ThorQ::ApiEndpoints::FriendRequestEndpoint::handleMessageRequest(ThorQ::HandlerContext& context)
{

}

void ThorQ::ApiEndpoints::FriendRequestEndpoint::handleMessageIncomingAccept(ThorQ::HandlerContext& context)
{

}

void ThorQ::ApiEndpoints::FriendRequestEndpoint::handleMessageIncomingDeny(ThorQ::HandlerContext& context)
{

}

void ThorQ::ApiEndpoints::FriendRequestEndpoint::handleMessageOutgoingRemove(ThorQ::HandlerContext& context)
{

}
