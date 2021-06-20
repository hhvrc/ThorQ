#ifndef ENDPOINT_MESSAGE_H
#define ENDPOINT_MESSAGE_H

#include <typedefs_global.h>
#include <typedefs_server.h>

namespace ThorQ::ApiEndpoints::MessageEndpoint {
void handleMessage(HandlerContext& context);
}

#endif // ENDPOINT_MESSAGE_H
