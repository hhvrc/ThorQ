#ifndef ENDPOINT_USER_H
#define ENDPOINT_USER_H

#include <typedefs_global.h>
#include <typedefs_server.h>

namespace ThorQ::ApiEndpoints::UserEndpoint {
void handleMessage(HandlerContext& context);
void handleMessageGet(HandlerContext& context);
void handleMessageBlock(HandlerContext& context);
void handleMessageUnBlock(HandlerContext& context);
}

#endif // ENDPOINT_USER_H
