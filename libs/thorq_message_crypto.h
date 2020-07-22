#ifndef MSG_CRYPTO_H
#define MSG_CRYPTO_H

#include "thorq_message.h"

#define THORQ_MSG_MAX_CRYPTO_DATA_LEN THORQ_MSG_MAX_PAYLOAD_LEN - 1

typedef struct __thorq_crypto
{
	std::uint8_t mode;
	std::uint8_t data[THORQ_MSG_MAX_CRYPTO_DATA_LEN];
} thorq_crypto_t;

#endif // MSG_CRYPTO_H
