#include "endpoint_group.h"

#include "apiserver_connection.h"
#include "messagehandlingcontext.h"

#include <schemas_common.h>
#include <fmt/core.h>

void ThorQ::ApiEndpoints::GroupEndpoint::handleMessage(HandlerContext& context)
{
    auto msgBody = context.body<ThorQ::Serialization::Group::Message>();
    auto subBody = msgBody->body();
    auto connection = context.apiConnection();

    fmt::print("[MSG] Group\n");

    if (subBody == nullptr) {
        connection->disconnect();
        return;
    }

    context.setBody(subBody);

    switch (msgBody->body_type()) {
    case ThorQ::Serialization::Group::Body_get:
        handleMessageGet(context);
        break;
    case ThorQ::Serialization::Group::Body_create:
        handleMessageCreate(context);
        break;
    case ThorQ::Serialization::Group::Body_destroy:
        handleMessageDestroy(context);
        break;
    case ThorQ::Serialization::Group::Body_leave:
        handleMessageLeave(context);
        break;
    case ThorQ::Serialization::Group::Body_user_kick:
        handleMessageUserKick(context);
        break;
    case ThorQ::Serialization::Group::Body_user_invite:
        handleMessageUserInvite(context);
        break;
    case ThorQ::Serialization::Group::Body_invite_accept:
        handleMessageInviteAccept(context);
        break;
    case ThorQ::Serialization::Group::Body_invite_reject:
        handleMessageInviteReject(context);
        break;
    case ThorQ::Serialization::Group::Body_set_name:
        handleMessageSetImage(context);
        break;
    case ThorQ::Serialization::Group::Body_set_image:
        handleMessageSetImage(context);
        break;
    default:
        fmt::print("[MSG][GROUP] Invalid\n");
        // TODO: THORQ_DISCONNECT_REASON::INVALID_REQUEST
        connection->disconnect();
        break;
    }
}

void ThorQ::ApiEndpoints::GroupEndpoint::handleMessageGet(ThorQ::HandlerContext& context)
{

}

void ThorQ::ApiEndpoints::GroupEndpoint::handleMessageCreate(ThorQ::HandlerContext& context)
{

}

void ThorQ::ApiEndpoints::GroupEndpoint::handleMessageDestroy(ThorQ::HandlerContext& context)
{

}

void ThorQ::ApiEndpoints::GroupEndpoint::handleMessageLeave(ThorQ::HandlerContext& context)
{

}

void ThorQ::ApiEndpoints::GroupEndpoint::handleMessageUserKick(ThorQ::HandlerContext& context)
{

}

void ThorQ::ApiEndpoints::GroupEndpoint::handleMessageUserInvite(ThorQ::HandlerContext& context)
{

}

void ThorQ::ApiEndpoints::GroupEndpoint::handleMessageInviteAccept(ThorQ::HandlerContext& context)
{

}

void ThorQ::ApiEndpoints::GroupEndpoint::handleMessageInviteReject(ThorQ::HandlerContext& context)
{

}

void ThorQ::ApiEndpoints::GroupEndpoint::handleMessageSetName(ThorQ::HandlerContext& context)
{

}

void ThorQ::ApiEndpoints::GroupEndpoint::handleMessageSetImage(ThorQ::HandlerContext& context)
{

}
