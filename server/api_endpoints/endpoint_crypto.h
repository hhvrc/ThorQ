#ifndef ENDPOINT_CRYPTO_H
#define ENDPOINT_CRYPTO_H

#include <typedefs_global.h>
#include <typedefs_server.h>
#include <cryptography/encryption.h>

namespace ThorQ::ApiEndpoints::CryptoEndpoint {
void handleMessage(HandlerContext& context);
}

#endif // ENDPOINT_CRYPTO_H
