/// @file thorq_payload_announcement.h
///
///

#ifndef THORQ_PAYLOAD_ANNOUNCEMENT_H
#define THORQ_PAYLOAD_ANNOUNCEMENT_H

#include <vector>

#include <QString>

#include "enums.h"

/// @enum THORQ_PAYLOAD_ANNOUNCEMENT_TYPE
enum THORQ_PAYLOAD_ANNOUNCEMENT_TYPE : std::uint8_t
{
    THORQ_PAYLOAD_ANNOUNCEMENT_TYPE_ADMIN,
    THORQ_PAYLOAD_ANNOUNCEMENT_TYPE_SYSTEM,
};

/// @enum THORQ_PAYLOAD_ANNOUNCEMENT_REASON
enum THORQ_PAYLOAD_ANNOUNCEMENT_REASON : std::uint8_t
{
    THORQ_PAYLOAD_ANNOUNCEMENT_REASON_ALERT,
    THORQ_PAYLOAD_ANNOUNCEMENT_REASON_NOTICE,
    THORQ_PAYLOAD_ANNOUNCEMENT_REASON_MAINTANENCE,
};

/**
 * @brief thorq_payload_announcement_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_announcement_is_valid(const std::vector<std::uint8_t>& payload)
{
    return payload.size() > 3
        && payload[0] == THORQ_PAYLOAD_ID_ANNOUNCEMENT
        && payload[1] <= THORQ_PAYLOAD_ANNOUNCEMENT_TYPE_SYSTEM // Max enum value
        && payload[2] <= THORQ_PAYLOAD_ANNOUNCEMENT_REASON_MAINTANENCE; // Max enum value
}

/**
 * @brief thorq_payload_announcement_pack
 * @param payload
 * @param type
 * @param reason
 * @param message
 */
inline void thorq_payload_announcement_pack(std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_ANNOUNCEMENT_TYPE type, THORQ_PAYLOAD_ANNOUNCEMENT_REASON reason, const QString& message)
{
    payload.resize(3 + message.size());
    payload[0] = THORQ_PAYLOAD_ID_ANNOUNCEMENT;
    payload[1] = static_cast<std::uint8_t>(type);
    payload[2] = static_cast<std::uint8_t>(reason);

    memcpy(payload.data() + 3, message.data(), message.size());
}

/**
 * @brief thorq_payload_announcement_get_type
 * @param payload
 * @param type
 */
inline void thorq_payload_announcement_get_type(const std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_ANNOUNCEMENT_TYPE& type)
{
    // if u did safety checking, this is fine
    type = static_cast<THORQ_PAYLOAD_ANNOUNCEMENT_TYPE>(payload[1]);
}

/**
 * @brief thorq_payload_announcement_get_reason
 * @param payload
 * @param reason
 */
inline void thorq_payload_announcement_get_reason(const std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_ANNOUNCEMENT_REASON& reason)
{
    // if u did safety checking, this is fine
    reason = static_cast<THORQ_PAYLOAD_ANNOUNCEMENT_REASON>(payload[2]);
}

/**
 * @brief thorq_payload_announcement_get_message
 * @param payload
 * @param message
 */
inline void thorq_payload_announcement_get_message(const std::vector<std::uint8_t>& payload, QString& message)
{
    message.resize((int)payload.size() - 3);

    memcpy(message.data(), payload.data() + 3, payload.size() - 3);
}

#endif // THORQ_PAYLOAD_ANNOUNCEMENT_H
