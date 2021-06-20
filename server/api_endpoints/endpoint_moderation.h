#ifndef ENDPOINT_MODERATION_H
#define ENDPOINT_MODERATION_H

#include <typedefs_global.h>
#include <typedefs_server.h>

namespace ThorQ::ApiEndpoints::ModerationEndpoint {
void handleMessage(HandlerContext& context);
}

#endif // ENDPOINT_MODERATION_H
