/// @file thorq_payload_announcement.h
///
///

#ifndef THORQ_PAYLOAD_ANNOUNCEMENT_H
#define THORQ_PAYLOAD_ANNOUNCEMENT_H

#include <vector>

#include "enums.h"
#include "serialization.h"

/// @enum THORQ_PAYLOAD_ANNOUNCEMENT
enum THORQ_PAYLOAD_ANNOUNCEMENT : std::uint8_t
{
    THORQ_PAYLOAD_ANNOUNCEMENT_ADMIN_MESSAGE,
    THORQ_PAYLOAD_ANNOUNCEMENT_SERVER_ALERT,
    THORQ_PAYLOAD_ANNOUNCEMENT_SERVER_NOTICE,
    THORQ_PAYLOAD_ANNOUNCEMENT_SERVER_MAINTANENCE,
};

/**
 * @brief thorq_payload_announcement_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_announcement_is_valid(const std::vector<std::uint8_t>& payload)
{
    return thorq_payload_serialization_is_valid(payload)
        && payload[0] == THORQ_PAYLOAD_ID_ANNOUNCEMENT
        && payload[1] <= THORQ_PAYLOAD_ANNOUNCEMENT_SERVER_MAINTANENCE; // Max enum value
}

/**
 * @brief thorq_payload_announcement_pack
 * @param payload
 * @param type
 * @param message
 */
inline void thorq_payload_announcement_pack(std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_ANNOUNCEMENT type, const std::string& message)
{
    thorq_payload_serialization_pack_1string(payload, THORQ_PAYLOAD_ID_ANNOUNCEMENT, type, message);
}

/**
 * @brief thorq_payload_announcement_unpack
 * @param payload
 * @param type
 * @param message
 */
inline void thorq_payload_announcement_unpack(const std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_ANNOUNCEMENT& type, std::string& message)
{
    type = static_cast<THORQ_PAYLOAD_ANNOUNCEMENT>(thorq_payload_serialization_get_cmd(payload));
    thorq_payload_serialization_unpack_1string(payload, message);
}

#endif // THORQ_PAYLOAD_ANNOUNCEMENT_H
