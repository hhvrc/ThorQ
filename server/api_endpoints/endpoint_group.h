#ifndef ENDPOINT_GROUP_H
#define ENDPOINT_GROUP_H

#include <typedefs_global.h>
#include <typedefs_server.h>
#include <cryptography/encryption.h>

#include <mutex>
#include <atomic>
#include <vector>
#include <memory>
#include <cstdint>

namespace ThorQ::ApiEndpoints::GroupEndpoint {
void handleMessage(HandlerContext& context);
void handleMessageGet(HandlerContext& context);
void handleMessageCreate(HandlerContext& context);
void handleMessageDestroy(HandlerContext& context);
void handleMessageLeave(HandlerContext& context);
void handleMessageUserKick(HandlerContext& context);
void handleMessageUserInvite(HandlerContext& context);
void handleMessageInviteAccept(HandlerContext& context);
void handleMessageInviteReject(HandlerContext& context);
void handleMessageSetName(HandlerContext& context);
void handleMessageSetImage(HandlerContext& context);
}

#endif // ENDPOINT_GROUP_H
