#include "endpoint_moderation.h"

#include "apiserver_connection.h"
#include "messagehandlingcontext.h"

#include <schemas_common.h>
#include <fmt/core.h>

void ThorQ::ApiEndpoints::ModerationEndpoint::handleMessage(ThorQ::HandlerContext& context)
{
    auto msgBody = context.body<ThorQ::Serialization::Moderation::Message>();
    auto subBody = msgBody->body();
    auto connection = context.apiConnection();

    fmt::print("[MSG] Moderation\n");

    if (subBody == nullptr) {
        connection->disconnect();
        return;
    }

    context.setBody(subBody);

    switch (msgBody->body_type()) {
    case ThorQ::Serialization::Moderation::Body_ban:
        break;
    case ThorQ::Serialization::Moderation::Body_report:
        break;
    default:
        fmt::print("[MSG][MODERATION] Invalid\n");
        // TODO: THORQ_DISCONNECT_REASON::INVALID_REQUEST
        connection->disconnect();
        break;
    }
}
