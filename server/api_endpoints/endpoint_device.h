#ifndef ENDPOINT_DEVICE_H
#define ENDPOINT_DEVICE_H

#include <typedefs_global.h>
#include <typedefs_server.h>

namespace ThorQ::ApiEndpoints::DeviceEndpoint {
void handleMessage(HandlerContext& context);
}

#endif // ENDPOINT_DEVICE_H
