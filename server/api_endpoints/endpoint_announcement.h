#ifndef ENDPOINT_ANNOUNCEMENT_H
#define ENDPOINT_ANNOUNCEMENT_H

#include <typedefs_global.h>
#include <typedefs_server.h>

namespace ThorQ::ApiEndpoints::AnnouncementEndpoint {
void handleMessage(HandlerContext& context);
}

#endif // ENDPOINT_ANNOUNCEMENT_H
