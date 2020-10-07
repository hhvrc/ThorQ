/// @file thorq_payload_message.h
///
///

#ifndef THORQ_MESSAGE_H
#define THORQ_MESSAGE_H

#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>

#include "constants.h"
#include "crypto.h"
#include "enums.h"

#ifdef WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif

/// Flags to describe the state of a message (these flags will NOT be encrypted)
enum THORQ_MESSAGE_FLAG : std::uint8_t
{
    THORQ_MESSAGE_FLAG_ENCRYPTED  = 1 << 0, ///< The following data is encrypted, it needs to get decrypted to make sense
    THORQ_MESSAGE_FLAG_RESERVED_2 = 1 << 1,
    THORQ_MESSAGE_FLAG_RESERVED_3 = 1 << 2,
    THORQ_MESSAGE_FLAG_RESERVED_4 = 1 << 3,
    THORQ_MESSAGE_FLAG_RESERVED_5 = 1 << 4,
    THORQ_MESSAGE_FLAG_RESERVED_6 = 1 << 5,
    THORQ_MESSAGE_FLAG_RESERVED_7 = 1 << 6,
    THORQ_MESSAGE_FLAG_RESERVED_8 = 1 << 7,
};

/* Fixed size of message
 * PAYLOAD | SIZE | CRYPTO_IV | MESSAGE_FLAGS
 */
constexpr std::size_t THORQ_MESSAGE_LEN =
        THORQ_PAYLOAD_CAP
      + sizeof(std::uint32_t)
      + CRYPTO_AES_IV_LEN
      + sizeof(THORQ_MESSAGE_FLAG);

#pragma pack(push, 1)
struct THORQ_PAYLOAD
{
    THORQ_PAYLOAD_ID id;
    union
    {
        std::uint8_t raw[THORQ_PAYLOAD_CAP];
    } data;
};
#pragma pack(pop)

typedef std::uint8_t THORQ_MESSAGE[
                                   sizeof(THORQ_MESSAGE_FLAG) +
                                   sizeof(std::uint32_t) +
                                   THORQ_PAYLOAD_CAP +
                                   CRYPTO_AES_IV_LEN
                                  ];

typedef std::uint8_t THORQ_MESSAGE[THORQ_MESSAGE_LEN];

inline bool thorq_message_encode(const THORQ_PAYLOAD& payload, std::size_t payloadLen, THORQ_MESSAGE& messageOut)
{
    if (payloadLen > THORQ_PAYLOAD_CAP)
        return false;

    // Set header
    messageOut[0] = 0;

    // Copy data
    memcpy(messageOut + 5, &payload, payloadLen);

    // Randomize all data after the payload (+iv)
    ThorQ::Crypto::RandomizeBytes(messageOut + 5 + payloadLen, THORQ_PAYLOAD_CAP + CRYPTO_AES_IV_LEN - payloadLen);

    // Convert and copy size
    payloadLen = htonl(payloadLen);
    memcpy(messageOut + 1, &payloadLen, sizeof(payloadLen));

	return true;
}
inline bool thorq_message_encode(const THORQ_PAYLOAD& payload, std::size_t payloadLen, THORQ_MESSAGE& messageOut, ThorQ::Crypto* crypto)
{
    if (payloadLen > THORQ_PAYLOAD_CAP)
		return false;

    // Set header
    messageOut[0] = THORQ_MESSAGE_FLAG_ENCRYPTED;

    // Encrpyt the data, this will copy it and the generated IV into messageOut
    if (!crypto->encrypt(messageOut + 5, (const std::uint8_t*)&payload, payloadLen, messageOut + 5 + THORQ_PAYLOAD_CAP))
        return false;

    // Randomize all data after the payload
    ThorQ::Crypto::RandomizeBytes(messageOut + 5 + payloadLen, THORQ_PAYLOAD_CAP - payloadLen);

    // Convert and copy size
    payloadLen = htonl(payloadLen);
    memcpy(messageOut + 1, &payloadLen, sizeof(payloadLen));

	return true;
}
inline bool thorq_message_decode(const THORQ_MESSAGE& message, THORQ_PAYLOAD& payloadOut, ThorQ::Crypto* crypto)
{
    std::uint32_t messageLen = 0;
    memcpy(&messageLen, message + 1, sizeof(messageLen));

    if ((message[0] & THORQ_MESSAGE_FLAG_ENCRYPTED) != 0)
    {
        if (!crypto->decrypt((std::uint8_t*)&payloadOut, message + 5, messageLen, message + 5 + THORQ_PAYLOAD_CAP))
            return false;
    }
    else
    {
        memcpy(&payloadOut, message + 5, messageLen);
    }

    if (message[0] > THORQ_PAYLOAD_ID__MAX)
        return false;

	return true;
}

#endif // THORQ_MESSAGE_H
