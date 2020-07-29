#ifndef THORQ_MESSAGE_H
#define THORQ_MESSAGE_H

#include <vector>
#include <cstdint>
#include <cstring>

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

constexpr std::size_t THORQ_MESSAGE_LEN = 3 + THORQ_PAYLOAD_LEN + THORQ_CRYPTO_CIPHER_IV_LEN;

inline bool thorq_message_is_valid(const std::vector<std::uint8_t>& data)
{
	if (data.size() != THORQ_MESSAGE_LEN)
		return false;

	std::uint16_t payloadSize = 0;
	payloadSize |= data[1] << 8;
	payloadSize |= data[2] << 0;

	return payloadSize <= THORQ_PAYLOAD_LEN;
}
inline bool thorq_message_is_valid(const std::uint8_t* data, std::size_t size)
{
	if (data == nullptr || size != THORQ_MESSAGE_LEN)
		return false;

	std::uint16_t payloadSize = 0;
	payloadSize |= data[1] << 8;
	payloadSize |= data[2] << 0;

	return payloadSize <= THORQ_PAYLOAD_LEN;
}
inline bool thorq_message_is_encrypted(const std::uint8_t* data, std::size_t size)
{
	return thorq_message_is_valid(data, size) && (data[0] & THORQ_MESSAGE_FLAG_ENCRYPTED) != 0;
}
inline bool thorq_message_is_encrypted(const std::vector<std::uint8_t>& data)
{
	return thorq_message_is_valid(data) && (data[0] & THORQ_MESSAGE_FLAG_ENCRYPTED) != 0;
}

inline void thorq_message_encode(const std::vector<std::uint8_t>& message, std::vector<std::uint8_t>& data)
{
	data.resize(THORQ_MESSAGE_LEN);

	// Set header
	data[0] = 0;

	std::uint16_t payloadSize = std::min(message.size(), THORQ_PAYLOAD_LEN);

	// Set size
	data[1] = payloadSize >> 8;
	data[2] = payloadSize >> 0;

	// Copy over payload
	memcpy(&data[3], &message[0], payloadSize);

	// Randomize the rest of the data
	ThorQ::Crypto::RandomizeBytes(&data[3 + payloadSize], THORQ_MESSAGE_LEN - (3 + payloadSize));
}
inline void thorq_message_encode(const std::vector<std::uint8_t>& message, std::vector<std::uint8_t>& data, ThorQ::Crypto* crypto)
{
	data.reserve(THORQ_MESSAGE_LEN);
	data.resize(THORQ_PAYLOAD_LEN);

	std::uint16_t payloadSize = std::min(message.size(), THORQ_PAYLOAD_LEN);

	memcpy(&data[0], &message[0], payloadSize);
	ThorQ::Crypto::RandomizeBytes(&data[payloadSize], THORQ_PAYLOAD_LEN - payloadSize);

	// Encrpytion
	{
		std::uint8_t iv[THORQ_CRYPTO_CIPHER_IV_LEN];
		crypto->encrypt(data, &iv[0]);

		data.resize(THORQ_MESSAGE_LEN);
		memcpy(&data[3], &data[0], THORQ_PAYLOAD_LEN);
		memcpy(&data[3 + THORQ_PAYLOAD_LEN], iv, THORQ_CRYPTO_CIPHER_IV_LEN);
	}

	// Set header
	data[0] = THORQ_MESSAGE_FLAG_ENCRYPTED;

	// Set size
	data[1] = payloadSize >> 8;
	data[2] = payloadSize >> 0;
}
inline void thorq_message_decode(const std::uint8_t* data, std::size_t dataSize, std::vector<std::uint8_t>& message, ThorQ::Crypto* crypto)
{
	(void)dataSize;

	std::uint16_t payloadSize = 0;
	payloadSize |= data[1] << 8;
	payloadSize |= data[2] << 0;

	if ((data[0] & THORQ_MESSAGE_FLAG_ENCRYPTED) != 0)
	{
		std::uint8_t iv[THORQ_CRYPTO_CIPHER_IV_LEN];
		memcpy(&iv[0], &data[3 + THORQ_PAYLOAD_LEN], THORQ_CRYPTO_CIPHER_IV_LEN);

		message.resize(THORQ_PAYLOAD_LEN);
		memcpy(&message[0], &data[3], THORQ_PAYLOAD_LEN);

		crypto->decrypt(message, iv);
		message.resize(payloadSize);
	}
	else
	{
		message.resize(payloadSize);
		memcpy(&message[0], &data[3], payloadSize);
	}
}

#endif // THORQ_MESSAGE_H
