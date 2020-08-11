#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <string>
#include "thorq_payload.h"

/** @typedef thorq_notification_type_t
 */
typedef enum {
    THORQ_NOTIFICATION_USER_STATUS,
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
    return payload.id == THORQ_PAYLOAD_ID_NOTIFICATION;
}

/**
 * @brief thorq_payload_notification_pack
 * @param payload
 * @param type
 * @param data
 */
inline void thorq_payload_notification_pack(thorq_payload_t& payload, const thorq_notification_type_t& type, const std::string& data)
{
    payload.id = THORQ_PAYLOAD_ID_EVENT;
    payload.data.resize(1 + data.size());
    payload.data[0] = static_cast<std::uint8_t>(type);
    if (data.size() != 0)
        memcpy(&payload.data[1], &data[0], data.size());
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

#endif // NOTIFICATION_H
