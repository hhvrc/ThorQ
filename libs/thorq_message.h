#ifndef MSG_MESSAGE_H
#define MSG_MESSAGE_H

#include <cstdint>
#include <memory>
#include "constants.h"
#include "crypto.h"
#include "enums.h"

// Macro to define packed structures
#ifdef __GNUC__
  #define THORQPACKED( __Declaration__ ) __Declaration__ __attribute__((packed))
#else
  #define THORQPACKED( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#endif

/**
 * @brief The thorq_message_t struct
 */
typedef struct __thorq_msg
{
	__thorq_msg(){};
	__thorq_msg(const __thorq_msg& other) = delete;
	__thorq_msg& operator = (const __thorq_msg& other) = delete;

    std::uint8_t flags = 0;
	std::vector<std::uint8_t> payload; ///< First byte is the message id
	std::uint8_t              payload_iv[THORQ_CRYPTO_CIPHER_IV_LEN]{0};

	inline bool operator == (const __thorq_msg& other) const
	{
		return flags == other.flags &&
			   payload == other.payload &&
			   (memcmp(payload_iv, other.payload_iv, THORQ_CRYPTO_CIPHER_IV_LEN) == 0);
	}
	inline bool operator != (const __thorq_msg& other) const
	{
		return !(*this == other);
	}
} thorq_msg_t;

constexpr std::size_t THORQ_MSG_SIZE_MAX = 1 + THORQ_MSG_MAX_PAYLOAD_LEN + THORQ_CRYPTO_CIPHER_IV_LEN;

static inline bool thorq_msg_is_valid(const std::uint8_t* data, std::size_t size)
{
	if (data == nullptr || size <= 1 || size > THORQ_MSG_SIZE_MAX)
		return false;

	if ((data[0] & THORQ_MSG_FLAG_ENCRYPTED) == 0)
		return true;

	return size > 1 + THORQ_CRYPTO_CIPHER_IV_LEN;
}
static inline bool thorq_msg_is_valid(const std::vector<std::uint8_t>* msg)
{
	return thorq_msg_is_valid(msg->data(), msg->size());
}
static inline bool thorq_msg_is_encrypted(const thorq_msg_t* msg)
{
	return (msg->flags & THORQ_MSG_FLAG_ENCRYPTED) != 0;
}

static inline bool thorq_msg_encode(const thorq_msg_t* msg, std::vector<std::uint8_t>* data)
{
	if (msg->payload.size() == 0)
		return false;

	if (thorq_msg_is_encrypted(msg))
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
static inline bool thorq_msg_decode(const std::uint8_t* data, std::size_t size, thorq_msg_t* msg)
{
	if (data == nullptr || size <= 1 || size > THORQ_MSG_SIZE_MAX)
		return false;

	msg->flags = data[0];
	msg->payload.clear();

	if (thorq_msg_is_encrypted(msg))
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

static inline bool thorq_msg_encrypt(thorq_msg_t* msg, ThorQ::Crypto* crypto)
{
	if (!thorq_msg_is_encrypted(msg))
	{
		if (!crypto->encrypt(msg->payload, msg->payload_iv))
			return false;
		msg->flags |= THORQ_MSG_FLAG_ENCRYPTED;
	}
	return true;
}
static inline bool thorq_msg_decrypt(thorq_msg_t* msg, ThorQ::Crypto* crypto)
{
	if (thorq_msg_is_encrypted(msg))
	{
		if (!crypto->ready() || !crypto->decrypt(msg->payload, msg->payload_iv))
			return false;

		msg->flags &= ~THORQ_MSG_FLAG_ENCRYPTED;
	}
	return true;
}

static inline thorq_msg_id_t thorq_msg_get_msg_id(const thorq_msg_t* msg)
{
	if (msg->payload.size() < 1 || thorq_msg_is_encrypted(msg))
		return THORQ_MSG_ID_INVALID;

	return (thorq_msg_id_t)msg->payload.data()[0];
}

#endif // MSG_MESSAGE_H
