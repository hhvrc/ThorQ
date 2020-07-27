#ifndef THORQ_PAYLOAD_H
#define THORQ_PAYLOAD_H

#include <vector>
#include <cstdint>
#include <enums.h>
#include <cstring>

// Types:

// Message
//    Payload
//          Version
//       Crypto
//       Auth
//       Heartbeat
//          Command
//          Command_ack
//          Notification
//          Collar

typedef struct __thorq_payload
{
	thorq_payload_id_t id;
	std::vector<std::uint8_t> data;

	inline bool operator==(const struct __thorq_payload& other) const
	{
		return id == other.id && data == other.data;
	}
	inline bool operator!=(const struct __thorq_payload& other) const
	{
		return !(*this == other);
	}
} thorq_payload_t;

inline thorq_payload_id_t thorq_payload_get_id(const thorq_payload_t& payload)
{
    return payload.id;
}

inline void thorq_payload_pack(const thorq_payload_t& payload, std::vector<std::uint8_t>& message)
{
	message.resize(1 + payload.data.size());

	// Copy the payload id
	message[0] = (std::uint8_t)payload.id;

	// Copy the payload data
	memcpy(&message[1], &payload.data[0], payload.data.size());
}

inline void thorq_payload_unpack(const std::vector<std::uint8_t>& message, thorq_payload_t& payload)
{
	payload.id = (thorq_payload_id_t)message[0];
	payload.data.resize(message.size() - 1);

	memcpy(&payload.data[0], &message[1], payload.data.size());
}

inline thorq_payload_id_t thorq_payload_get_id(const std::vector<std::uint8_t>& message)
{
	if (message.size() < 1)
		return THORQ_PAYLOAD_ID_INVALID;

	return (thorq_payload_id_t)message[0];
}

#endif // THORQ_PAYLOAD_H
