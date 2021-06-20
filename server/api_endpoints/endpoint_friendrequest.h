#ifndef ENDPOINT_FRIENDREQUEST_H
#define ENDPOINT_FRIENDREQUEST_H

#include <typedefs_global.h>
#include <typedefs_server.h>

namespace ThorQ::ApiEndpoints::FriendRequestEndpoint {
void handleMessage(HandlerContext& context);
void handleMessageRequest(HandlerContext& context);
void handleMessageIncomingAccept(HandlerContext& context);
void handleMessageIncomingDeny(HandlerContext& context);
void handleMessageOutgoingRemove(HandlerContext& context);
}

#endif // ENDPOINT_FRIENDREQUEST_H
