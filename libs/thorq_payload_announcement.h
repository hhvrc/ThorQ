#ifndef THORQ_MSG_PAYLOAD_ANNOUNCEMENT_H
#define THORQ_MSG_PAYLOAD_ANNOUNCEMENT_H

#include <string>
#include <vector>

#include "enums.h"

typedef enum {
    ADMIN,
    SYSTEM,
} thorq_announcement_type_t;

typedef enum {
    ALERT,
    NOTICE,
    MAINTANENCE,
} thorq_announcement_reason_t;

/**
 * @brief thorq_message_announcement_is_valid
 * @param payload
 * @return
 */
inline bool thorq_message_announcement_is_valid(const std::vector<std::uint8_t>& payload)
{
    return payload.size() > 3 && payload[0] == THORQ_MESSAGE_ID_ANNOUNCEMENT;
}

/**
 * @brief thorq_announcement_pack
 * @param payload
 * @param type
 * @param reason
 * @param message
 */
inline void thorq_announcement_pack(std::vector<std::uint8_t>& payload, thorq_announcement_type_t type, thorq_announcement_reason_t reason, const std::string& message)
{
    payload.resize(3 + message.size());
    payload[0] = THORQ_MESSAGE_ID_ANNOUNCEMENT;
    payload[1] = static_cast<std::uint8_t>(type);
    payload[2] = static_cast<std::uint8_t>(reason);

    memcpy(payload.data() + 3, message.data(), message.size());
}

/**
 * @brief thorq_msg_announcement_get_type
 * @param payload
 * @param type
 */
inline void thorq_message_announcement_get_type(const std::vector<std::uint8_t>& payload, thorq_announcement_type_t& type)
{
    type = static_cast<thorq_announcement_type_t>(payload[1]);
}

/**
 * @brief thorq_msg_announcement_get_reason
 * @param payload
 * @param reason
 */
inline void thorq_message_announcement_get_reason(const std::vector<std::uint8_t>& payload, thorq_announcement_reason_t& reason)
{
    reason = static_cast<thorq_announcement_reason_t>(payload[2]);
}

/**
 * @brief thorq_msg_announcement_get_message
 * @param payload
 * @param message
 */
inline void thorq_message_announcement_get_message(const std::vector<std::uint8_t>& payload, std::string& message)
{
    message.resize(payload.size() - 3);

    memcpy(message.data(), payload.data() + 3, payload.size() - 3);
}

#endif // THORQ_PAYLOAD_ANNOUNCEMENT_H
