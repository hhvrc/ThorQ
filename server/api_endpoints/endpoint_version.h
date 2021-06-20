#ifndef ENDPOINT_VERSION_H
#define ENDPOINT_VERSION_H

#include <typedefs_global.h>
#include <typedefs_server.h>

namespace ThorQ::ApiEndpoints::VersionEndpoint {
void handleMessage(HandlerContext& context);
}

#endif // ENDPOINT_VERSION_H
