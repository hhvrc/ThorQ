#include "endpoint_user.h"

#include "apiserver_connection.h"
#include "messagehandlingcontext.h"

#include <schemas_common.h>
#include <fmt/core.h>

void ThorQ::ApiEndpoints::UserEndpoint::handleMessage(ThorQ::HandlerContext& context)
{
    auto msgBody = context.body<ThorQ::Serialization::User::Message>();
    auto subBody = msgBody->body();
    auto connection = context.apiConnection();

    fmt::print("[MSG] User\n");

    if (subBody == nullptr) {
        connection->disconnect();
        return;
    }

    context.setBody(subBody);

    switch (msgBody->body_type()) {
    case ThorQ::Serialization::User::Body_get:
        ThorQ::ApiEndpoints::UserEndpoint::handleMessageGet(context);
        break;
    case ThorQ::Serialization::User::Body_block:
        ThorQ::ApiEndpoints::UserEndpoint::handleMessageBlock(context);
        break;
    case ThorQ::Serialization::User::Body_unblock:
        ThorQ::ApiEndpoints::UserEndpoint::handleMessageUnBlock(context);
        break;
    case ThorQ::Serialization::User::Body_user:
    default:
        fmt::print("[MSG][USER] Invalid\n");
        // TODO: THORQ_DISCONNECT_REASON::INVALID_REQUEST
        connection->disconnect();
        break;
    }
}

void ThorQ::ApiEndpoints::UserEndpoint::handleMessageGet(ThorQ::HandlerContext& context)
{
    auto msgBody = context.body<ThorQ::Serialization::Uuid>();
}

void ThorQ::ApiEndpoints::UserEndpoint::handleMessageBlock(ThorQ::HandlerContext& context)
{
    auto msgBody = context.body<ThorQ::Serialization::Uuid>();
}

void ThorQ::ApiEndpoints::UserEndpoint::handleMessageUnBlock(ThorQ::HandlerContext& context)
{
    auto msgBody = context.body<ThorQ::Serialization::Uuid>();
}
