#ifndef ENDPOINT_P2P_H
#define ENDPOINT_P2P_H

#include <typedefs_global.h>
#include <typedefs_server.h>

namespace ThorQ::ApiEndpoints::P2PEndpoint {
void handleMessage(HandlerContext& context);
void handleMessagePeer(HandlerContext& context);
void handleMessagePostLocal(HandlerContext& context);
}

#endif // ENDPOINT_P2P_H
