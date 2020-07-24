#ifndef THORQ_PAYLOAD_H
#define THORQ_PAYLOAD_H

#include "thorq_message.h"
#include "crypto.h"

typedef struct __thorq_payload
{
	thorq_payload_id_t id;
	std::uint8_t size;
	std::uint8_t data[THORQ_MAX_PAYLOAD_LEN - 2];

	inline bool operator==(const __thorq_payload& other) const
	{
		if (id != other.id || size != other.size)
			return false;

		if (size == 0)
			return true;

		return memcmp(data, &other.data, size) == 0;
	}
	inline bool operator!=(const __thorq_payload& other) const
	{
		return !(*this == other);
	}
} thorq_payload_t;

inline thorq_payload_id_t thorq_payload_get_id(const thorq_payload_t* payload)
{
	return payload->id;
}
inline void thorq_payload_pack(const thorq_payload_t* payload, thorq_message_t* msg)
{
	msg->payload.resize(THORQ_MAX_PAYLOAD_LEN);

	// Copy the payload id
	msg->payload[0] = (std::uint8_t)payload->id;

	// Copy size
	msg->payload[1] = payload->size;

	// Copy the payload data
	if (payload->size != 0)
		memcpy(msg->payload.data() + 1, payload->data, payload->size);

	std::uint16_t writtenSize = payload->size + 2;

	// Make the rest of the data seem random
	if (writtenSize < THORQ_MAX_PAYLOAD_LEN)
		ThorQ::Crypto::RandomizeBytes(msg->payload.data() + writtenSize, THORQ_MAX_PAYLOAD_LEN - writtenSize);
}
inline void thorq_payload_unpack(const thorq_message_t* msg, thorq_payload_t* payload)
{
	payload->id = (thorq_payload_id_t)msg->payload[0];
	payload->size = msg->payload[1];

	if (payload->size != 0)
		memcpy(payload->data, msg->payload.data(), payload->size);
}

#endif // THORQ_PAYLOAD_H
