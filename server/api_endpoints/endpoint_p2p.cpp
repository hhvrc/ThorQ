#include "endpoint_p2p.h"

#include "apiserver_connection.h"
#include "messagehandlingcontext.h"

#include <schemas_common.h>
#include <fmt/core.h>

void ThorQ::ApiEndpoints::P2PEndpoint::handleMessage(ThorQ::HandlerContext& context)
{
    auto msgBody = context.body<ThorQ::Serialization::Peer2Peer::Message>();
    auto subBody = msgBody->body();
    auto connection = context.apiConnection();

    fmt::print("[MSG] P2P\n");

    if (subBody == nullptr) {
        connection->disconnect();
        return;
    }

    context.setBody(subBody);

    switch (msgBody->body_type()) {
    case ThorQ::Serialization::Peer2Peer::Body_peer:
        handleMessagePeer(context);
        break;
    case ThorQ::Serialization::Peer2Peer::Body_post_local:
        handleMessagePostLocal(context);
        break;
    default:
        fmt::print("[MSG][P2P] Invalid\n");
        // TODO: THORQ_DISCONNECT_REASON::INVALID_REQUEST
        connection->disconnect();
        break;
    }
}

void ThorQ::ApiEndpoints::P2PEndpoint::handleMessagePeer(ThorQ::HandlerContext& context)
{

}

void ThorQ::ApiEndpoints::P2PEndpoint::handleMessagePostLocal(ThorQ::HandlerContext& context)
{

}
