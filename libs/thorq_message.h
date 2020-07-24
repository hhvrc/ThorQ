#ifndef MSG_MESSAGE_H
#define MSG_MESSAGE_H

#include <cstdint>
#include <memory>
#include "constants.h"
#include "crypto.h"
#include "enums.h"

/**
 * @brief The thorq_message_t struct
 */
typedef struct __thorq_message
{
	__thorq_message(){};
	__thorq_message(const __thorq_message& other) = delete;
	__thorq_message& operator = (const __thorq_message& other) = delete;

	std::uint8_t flags = 0;
	std::vector<std::uint8_t> payload; ///< Is vector to decrease amount of copies from converting from arrays to vectors, and back
	std::uint8_t              payload_iv[THORQ_CRYPTO_CIPHER_IV_LEN]{0};

	inline bool operator == (const __thorq_message& other) const
	{
		return flags == other.flags &&
			   payload == other.payload &&
			   (memcmp(payload_iv, other.payload_iv, THORQ_CRYPTO_CIPHER_IV_LEN) == 0);
	}
	inline bool operator != (const __thorq_message& other) const
	{
		return !(*this == other);
	}
} thorq_message_t;

constexpr std::size_t THORQ_MAX_MESSAGE_LEN = 1 + THORQ_MAX_PAYLOAD_LEN + THORQ_CRYPTO_CIPHER_IV_LEN;

inline bool thorq_message_is_valid(const std::uint8_t* data, std::size_t size)
{
	if (data == nullptr || size <= 1 || size > THORQ_MAX_MESSAGE_LEN)
		return false;

	if ((data[0] & THORQ_MSG_FLAG_ENCRYPTED) == 0)
		return true;

	return size > 1 + THORQ_CRYPTO_CIPHER_IV_LEN;
}
inline bool thorq_message_is_valid(const std::vector<std::uint8_t>* msg)
{
	return thorq_message_is_valid(msg->data(), msg->size());
}
inline bool thorq_message_is_encrypted(const thorq_message_t* msg)
{
	return (msg->flags & THORQ_MSG_FLAG_ENCRYPTED) != 0;
}

inline bool thorq_message_encode(const thorq_message_t* msg, std::vector<std::uint8_t>* data)
{
	if (msg->payload.size() == 0)
		return false;

	if (thorq_message_is_encrypted(msg))
	{
		data->resize(1 + msg->payload.size() + THORQ_CRYPTO_CIPHER_IV_LEN);

		memcpy(data->data() + 1 + msg->payload.size(), msg->payload_iv, THORQ_CRYPTO_CIPHER_IV_LEN);
	}
	else
	{
		data->resize(1 + msg->payload.size());
	}

	data->data()[0] = msg->flags;

	memcpy(data->data() + 1, msg->payload.data(), msg->payload.size());

	return true;
}
inline bool thorq_message_decode(const std::uint8_t* data, std::size_t size, thorq_message_t* msg)
{
	if (data == nullptr || size <= 1 || size > THORQ_MAX_MESSAGE_LEN)
		return false;

	msg->flags = data[0];
	msg->payload.clear();

	if (thorq_message_is_encrypted(msg))
	{
		if (size <= 1 + THORQ_CRYPTO_CIPHER_IV_LEN)
			return false;

		msg->payload.resize(size - 1 + THORQ_CRYPTO_CIPHER_IV_LEN);
		memcpy(msg->payload.data(), data + 1, msg->payload.size());
		memcpy(msg->payload_iv, data + 1 + msg->payload.size(), THORQ_CRYPTO_CIPHER_IV_LEN);
	}
	else
	{
		msg->payload.resize(size - 1);
		memcpy(msg->payload.data(), data + 1, msg->payload.size());
	}

	return true;
}

inline bool thorq_message_encrypt(thorq_message_t* msg, ThorQ::Crypto* crypto)
{
	if (!thorq_message_is_encrypted(msg))
	{
		if (!crypto->encrypt(msg->payload, msg->payload_iv))
			return false;
		msg->flags |= THORQ_MSG_FLAG_ENCRYPTED;
	}
	return true;
}
inline bool thorq_message_decrypt(thorq_message_t* msg, ThorQ::Crypto* crypto)
{
	if (thorq_message_is_encrypted(msg))
	{
		if (!crypto->ready() || !crypto->decrypt(msg->payload, msg->payload_iv))
			return false;

		msg->flags &= ~THORQ_MSG_FLAG_ENCRYPTED;
	}
	return true;
}

inline thorq_payload_id_t thorq_message_payload_id(const thorq_message_t* msg)
{
	if (msg->payload.size() < 1 || thorq_message_is_encrypted(msg))
		return THORQ_PAYLOAD_ID_INVALID;

	return (thorq_payload_id_t)msg->payload.data()[0];
}

#endif // MSG_MESSAGE_H
