#ifndef ENDPOINT_ACCOUNT_H
#define ENDPOINT_ACCOUNT_H

#include <typedefs_global.h>
#include <typedefs_server.h>

#include <vector>
#include <shared_mutex>
#include <atomic>
#include <cstdint>

namespace ThorQ::ApiEndpoints::AccountEndpoint {
void handleMessage(HandlerContext& context);
void handleMessageGetAccountId(HandlerContext& context);
void handleMessageGetHashingSalt(HandlerContext& context);
void handleMessageGetHashingParameters(HandlerContext& context);
void handleMessageLoginRequest(HandlerContext& context);
void handleMessageLogoutRequest(HandlerContext& context);
void handleMessageRegistrationRequest(HandlerContext& context);
void handleMessageRecoveryRequest(HandlerContext& context);
void handleMessageDeletionRequest(HandlerContext& context);
void handleMessageUpdateRequest(HandlerContext& context);
void handleMessageSetImageRequest(HandlerContext& context);
}

#endif // ENDPOINT_ACCOUNT_H
