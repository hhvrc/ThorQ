#ifndef ENDPOINT_FILE_H
#define ENDPOINT_FILE_H

#include <typedefs_global.h>
#include <typedefs_server.h>

namespace ThorQ::ApiEndpoints::FileEndpoint {
void handleMessage(HandlerContext& context);
}

#endif // ENDPOINT_FILE_H
