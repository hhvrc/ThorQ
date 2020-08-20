#ifndef THORQ_MESSAGE_H
#define THORQ_MESSAGE_H

#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>

#include "constants.h"
#include "crypto.h"
#include "enums.h"

typedef enum {
    THORQ_MESSAGE_FLAG_ENCRYPTED  = 1 << 0, ///< The following data is encrypted, it needs to get decrypted to make sense
    THORQ_MESSAGE_FLAG_RESERVED_2 = 1 << 1,
    THORQ_MESSAGE_FLAG_RESERVED_3 = 1 << 2,
    THORQ_MESSAGE_FLAG_RESERVED_4 = 1 << 3,
    THORQ_MESSAGE_FLAG_RESERVED_5 = 1 << 4,
    THORQ_MESSAGE_FLAG_RESERVED_6 = 1 << 5,
    THORQ_MESSAGE_FLAG_RESERVED_7 = 1 << 6,
    THORQ_MESSAGE_FLAG_RESERVED_8 = 1 << 7,
} thorq_message_header_flag_t; ///< Message entry flags to describe the state of a message

constexpr std::size_t THORQ_MESSAGE_LEN = THORQ_PAYLOAD_LEN + 4 + THORQ_CRYPTO_CIPHER_IV_LEN + 1;

inline bool thorq_message_encode(std::vector<std::uint8_t>& dataInOut)
{
    std::size_t messageSize = dataInOut.size();

    if (messageSize > THORQ_PAYLOAD_LEN)
		return false;

	dataInOut.resize(THORQ_MESSAGE_LEN);

	// Randomize the rest of the data
    ThorQ::Crypto::RandomizeBytes(&dataInOut[messageSize], THORQ_MESSAGE_LEN - messageSize);

    // Set size
    dataInOut[THORQ_PAYLOAD_LEN + 0] = (messageSize >> 24) & 0xFF;
    dataInOut[THORQ_PAYLOAD_LEN + 1] = (messageSize >> 16) & 0xFF;
    dataInOut[THORQ_PAYLOAD_LEN + 2] = (messageSize >>  8) & 0xFF;
    dataInOut[THORQ_PAYLOAD_LEN + 3] = (messageSize >>  0) & 0xFF;

	// Set header
	dataInOut[THORQ_MESSAGE_LEN - 1] = 0;

	return true;
}
inline bool thorq_message_encode(std::vector<std::uint8_t>& dataInOut, ThorQ::Crypto* crypto)
{
    std::size_t messageSize = dataInOut.size();

    if (messageSize > THORQ_PAYLOAD_LEN)
		return false;

	// Reserve space for the entire message, but keep the size as payload size for now
	dataInOut.reserve(THORQ_MESSAGE_LEN);
	dataInOut.resize(THORQ_PAYLOAD_LEN + 4);

	// Set size
    dataInOut[THORQ_PAYLOAD_LEN + 0] = (messageSize >> 24) & 0xFF;
    dataInOut[THORQ_PAYLOAD_LEN + 1] = (messageSize >> 16) & 0xFF;
    dataInOut[THORQ_PAYLOAD_LEN + 2] = (messageSize >>  8) & 0xFF;
    dataInOut[THORQ_PAYLOAD_LEN + 3] = (messageSize >>  0) & 0xFF;

    //Randomize all data after the message
    ThorQ::Crypto::RandomizeBytes(&dataInOut[messageSize], THORQ_PAYLOAD_LEN - messageSize);

	{
        // Encrpyt the data, this will copy the iv into the "iv" array
        std::uint8_t iv[THORQ_CRYPTO_CIPHER_IV_LEN];
		if (!crypto->encrypt(dataInOut, iv))
			return false;

        // resize data to the full message length after encryption
		dataInOut.resize(THORQ_MESSAGE_LEN);

        // Copy the iv to the end of the message
		memcpy(&dataInOut[THORQ_PAYLOAD_LEN + 4], iv, THORQ_CRYPTO_CIPHER_IV_LEN);
	}

	// Set header
	dataInOut[THORQ_MESSAGE_LEN - 1] = THORQ_MESSAGE_FLAG_ENCRYPTED;

	return true;
}
inline bool thorq_message_decode(std::vector<std::uint8_t>& dataInOut, ThorQ::Crypto* crypto)
{
	if (dataInOut.size() != THORQ_MESSAGE_LEN)
        return false;

	if ((dataInOut[THORQ_MESSAGE_LEN - 1] & THORQ_MESSAGE_FLAG_ENCRYPTED) != 0)
    {
        std::uint8_t iv[THORQ_CRYPTO_CIPHER_IV_LEN];
        memcpy(iv, &dataInOut[THORQ_PAYLOAD_LEN + 4], THORQ_CRYPTO_CIPHER_IV_LEN);

        dataInOut.resize(THORQ_PAYLOAD_LEN + 4);

        if (!crypto->decrypt(dataInOut, iv))
            return false;
    }

    std::uint32_t messageSize  = dataInOut[THORQ_PAYLOAD_LEN + 0] << 24;
    messageSize               |= dataInOut[THORQ_PAYLOAD_LEN + 1] << 16;
    messageSize               |= dataInOut[THORQ_PAYLOAD_LEN + 2] <<  8;
    messageSize               |= dataInOut[THORQ_PAYLOAD_LEN + 3] <<  0;

    if (messageSize > THORQ_PAYLOAD_LEN)
        return false;

    dataInOut.resize(messageSize);

	return true;
}

#endif // THORQ_MESSAGE_H
