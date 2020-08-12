#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <string>
#include "thorq_payload.h"

/** @typedef thorq_notification_type_t
 */
typedef enum {
    THORQ_NOTIFICATION_USER_ACTIVITY,

    THORQ_NOTIFICATION_USER_OFFLINE,
    THORQ_NOTIFICATION_USER_OFFLINE_LOS,
    THORQ_NOTIFICATION_USER_OFFLINE_TIMEOUT,
} thorq_notification_type_t;

/**
 * @brief thorq_payload_notification_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_notification_is_valid(const thorq_payload_t& payload)
{
	if (payload.data.size() == 0)
		return false;

	switch (payload.data[0]) {
	case THORQ_NOTIFICATION_USER_ACTIVITY:
		return payload.data.size() >= 3;
	case THORQ_NOTIFICATION_USER_OFFLINE:
	case THORQ_NOTIFICATION_USER_OFFLINE_LOS:
	case THORQ_NOTIFICATION_USER_OFFLINE_TIMEOUT:
		return payload.data.size() >= 2;
	default:
		return false;
	}
}

/**
 * @brief thorq_payload_notification_pack
 * @param payload
 * @param type
 * @param message
 */
inline void thorq_payload_notification_pack(thorq_payload_t& payload, const thorq_notification_type_t& type, const std::string& message)
{
    payload.id = THORQ_PAYLOAD_ID_EVENT;
    payload.data.resize(1 + message.size());
    payload.data[0] = static_cast<std::uint8_t>(type);
    if (message.size() != 0)
        memcpy(&payload.data[1], &message[0], message.size());
}

/**
 * @brief thorq_payload_notification_pack
 * @param payload
 * @param type
 * @param message
 * @param data
 */
inline void thorq_payload_notification_pack(thorq_payload_t& payload, const thorq_notification_type_t& type, const std::string& message, std::uint8_t data)
{
    payload.id = THORQ_PAYLOAD_ID_EVENT;
    payload.data.resize(1 + message.size() + 1);
    payload.data[0] = static_cast<std::uint8_t>(type);
    if (message.size() != 0)
        memcpy(&payload.data[1], &message[0], message.size());
    payload.data[message.size() + 1] = data;
}

inline void thorq_payload_notification_get_type(const thorq_payload_t& payload, thorq_notification_type_t& type)
{
    type = static_cast<thorq_notification_type_t>(payload.data[0]);
}

/**
 * @brief thorq_payload_notification_get_message
 * @param payload
 * @param message
 */
inline void thorq_payload_notification_get_message(const thorq_payload_t& payload, std::string& message)
{
    message.resize(payload.data.size() - 1);
    if (message.size() != 0)
        memcpy(&message[0], &payload.data[1], payload.data.size() - 1);
}

/**
 * @brief thorq_payload_notification_get_message
 * @param payload
 * @param message
 * @param data
 */
inline void thorq_payload_notification_get_message_and_data(const thorq_payload_t& payload, std::string& message, std::uint8_t data)
{
    message.resize(payload.data.size() - 2);
    if (message.size() != 0)
        memcpy(&message[0], &payload.data[1], payload.data.size() - 2);
    data = payload.data[payload.data.size() - 1];
}

#endif // NOTIFICATION_H
