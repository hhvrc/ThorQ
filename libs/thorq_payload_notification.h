#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <string>
#include <vector>

#include "enums.h"

/** @typedef thorq_notification_type_t
 */
typedef enum {
    THORQ_NOTIFICATION_USER_ACTIVITY,

    THORQ_NOTIFICATION_USER_OFFLINE,
    THORQ_NOTIFICATION_USER_OFFLINE_LOS,
    THORQ_NOTIFICATION_USER_OFFLINE_TIMEOUT,
} thorq_notification_type_t;

/**
 * @brief thorq_message_notification_is_valid
 * @param payload
 * @return
 */
inline bool thorq_message_notification_is_valid(const std::vector<std::uint8_t>& payload)
{
    if (payload.size() >= 2)
    {
        switch (payload[0]) {
        case THORQ_NOTIFICATION_USER_OFFLINE:
        case THORQ_NOTIFICATION_USER_OFFLINE_LOS:
        case THORQ_NOTIFICATION_USER_OFFLINE_TIMEOUT:
            return true;
        case THORQ_NOTIFICATION_USER_ACTIVITY:
            return payload.size() >= 3;
        default:
            return false;
        }
    }

    return false;
}

/**
 * @brief thorq_message_notification_pack
 * @param payload
 * @param type
 * @param message
 */
inline void thorq_message_notification_pack(std::vector<std::uint8_t>& payload, const thorq_notification_type_t& type, const std::string& message)
{
    payload.resize(2 + message.size());

    payload[0] = THORQ_MESSAGE_ID_NOTIFICATION;
    payload[1] = static_cast<std::uint8_t>(type);

    memcpy(payload.data() + 2, message.data(), message.size());
}

/**
 * @brief thorq_message_notification_pack
 * @param payload
 * @param type
 * @param message
 * @param data
 */
inline void thorq_message_notification_pack(std::vector<std::uint8_t>& payload, const thorq_notification_type_t& type, const std::string& message, std::uint8_t data)
{
    payload.resize(2 + message.size() + 1);

    payload[0] = THORQ_MESSAGE_ID_NOTIFICATION;
    payload[1] = static_cast<std::uint8_t>(type);

    memcpy(payload.data() + 2, message.data(), message.size());

    payload[2 + message.size()] = data;
}

inline void thorq_message_notification_get_type(const std::vector<std::uint8_t>& payload, thorq_notification_type_t& type)
{
    type = static_cast<thorq_notification_type_t>(payload[1]);
}

/**
 * @brief thorq_message_notification_get_message
 * @param payload
 * @param message
 */
inline void thorq_message_notification_get_message(const std::vector<std::uint8_t>& payload, std::string& message)
{
    message.resize(payload.size() - 2);

    memcpy(message.data(), payload.data() + 2, payload.size() - 2);
}

/**
 * @brief thorq_message_notification_get_message
 * @param payload
 * @param message
 * @param data
 */
inline void thorq_message_notification_get_message_and_data(const std::vector<std::uint8_t>& payload, std::string& message, std::uint8_t data)
{
    message.resize(payload.size() - 3);

    memcpy(message.data(), payload.data() + 2, message.size());

    data = payload[payload.size() - 2];
}

#endif // NOTIFICATION_H
