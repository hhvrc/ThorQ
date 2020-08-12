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
	THORQ_PAYLOAD_ID id;
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

/**
 * @brief thorq_payload_get_id
 * @param payload
 * @return
 */
inline THORQ_PAYLOAD_ID thorq_payload_get_id(const thorq_payload_t& payload)
{
    return payload.id;
}

/**
 * @brief thorq_payload_pack
 * @param payload
 * @param message
 */
inline void thorq_payload_pack(const thorq_payload_t& payload, std::vector<std::uint8_t>& message)
{
	message.resize(1 + payload.data.size());

	// Copy the payload id
	message[0] = (std::uint8_t)payload.id;

	// Copy the payload data
    if (payload.data.size() != 0)
        memcpy(&message[1], &payload.data[0], payload.data.size());
}

/**
 * @brief thorq_payload_unpack
 * @param message
 * @param payload
 */
inline void thorq_payload_unpack(const std::vector<std::uint8_t>& message, thorq_payload_t& payload)
{
    payload.data.resize(message.size() - 1);

    payload.id = (THORQ_PAYLOAD_ID)message[0];

    if (payload.data.size() != 0)
        memcpy(&payload.data[0], &message[1], payload.data.size());
}

/**
 * @brief thorq_payload_get_id
 * @param message
 * @return
 */
inline THORQ_PAYLOAD_ID thorq_payload_get_id(const std::vector<std::uint8_t>& message)
{
	if (message.size() < 1)
		return THORQ_PAYLOAD_ID_INVALID;

	return (THORQ_PAYLOAD_ID)message[0];
}

#endif // THORQ_PAYLOAD_H
