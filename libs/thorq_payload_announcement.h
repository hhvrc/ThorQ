#ifndef THORQ_MSG_PAYLOAD_ANNOUNCEMENT_H
#define THORQ_MSG_PAYLOAD_ANNOUNCEMENT_H

#include <string>

#include "thorq_payload.h"

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
 * @brief thorq_payload_announcement_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_announcement_is_valid(const thorq_payload_t& payload)
{
    return payload.id == THORQ_PAYLOAD_ID_ANNOUNCEMENT && payload.data.size() > 2;
}

/**
 * @brief thorq_announcement_pack
 * @param payload
 * @param type
 * @param reason
 * @param message
 */
inline void thorq_announcement_pack(thorq_payload_t& payload, thorq_announcement_type_t type, thorq_announcement_reason_t reason, const std::string& message)
{
    payload.id = THORQ_PAYLOAD_ID_ANNOUNCEMENT;
    payload.data.resize(message.size() + 2);
    payload.data[0] = static_cast<std::uint8_t>(type);
    payload.data[1] = static_cast<std::uint8_t>(reason);
    if (message.size() != 0)
        memcpy(&payload.data[0], &message[0], message.size());
}

/**
 * @brief thorq_msg_announcement_get_type
 * @param payload
 * @param type
 */
inline void thorq_payload_announcement_get_type(const thorq_payload_t& payload, thorq_announcement_type_t& type)
{
    type = static_cast<thorq_announcement_type_t>(payload.data[0]);
}

/**
 * @brief thorq_msg_announcement_get_reason
 * @param payload
 * @param reason
 */
inline void thorq_payload_announcement_get_reason(const thorq_payload_t& payload, thorq_announcement_reason_t& reason)
{
    reason = static_cast<thorq_announcement_reason_t>(payload.data[1]);
}

/**
 * @brief thorq_msg_announcement_get_message
 * @param payload
 * @param message
 */
inline void thorq_payload_announcement_get_message(const thorq_payload_t& payload, std::string& message)
{
    message.resize(payload.data.size() - 2);
    if (message.size() != 0)
        memcpy(&message[0], &payload.data[2], payload.data.size() - 2);
}

#endif // THORQ_PAYLOAD_ANNOUNCEMENT_H
