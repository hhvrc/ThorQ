#ifndef THORQ_PAYLOAD_EVENT_H
#define THORQ_PAYLOAD_EVENT_H

#include <QString>
#include <vector>

#include "enums.h"

/** @file thorq_payload_event.h
 *
 */

/** @typedef thorq_event_type_t
 *
 */
typedef enum {
    THORQ_EVENT_SESSION_REQUESTED,
    THORQ_EVENT_SESSION_STARTED,
    THORQ_EVENT_SESSION_STOPPED,
} thorq_event_type_t;

/**
 * @brief thorq_payload_event_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_event_is_valid(const std::vector<std::uint8_t>& payload)
{
    return payload.size() >= 2 && payload[0] == THORQ_PAYLOAD_ID_EVENT;
}

/**
 * @brief thorq_payload_event_pack
 * @param payload
 * @param type
 * @param data
 */
inline void thorq_payload_event_pack(std::vector<std::uint8_t>& payload, const thorq_event_type_t& type, const QString& data)
{
    payload.resize(2 + data.size());
    payload[0] = THORQ_PAYLOAD_ID_EVENT;
    payload[1] = static_cast<std::uint8_t>(type);

    memcpy(payload.data() + 2, data.data(), data.size());
}

/**
 * @brief thorq_payload_event_get_type
 * @param payload
 * @param type
 */
inline void thorq_payload_event_get_type(const std::vector<std::uint8_t>& payload, thorq_event_type_t& type)
{
    type = static_cast<thorq_event_type_t>(payload[1]);
}

/**
 * @brief thorq_payload_event_get_message
 * @param payload
 * @param message
 */
inline void thorq_payload_event_get_message(const std::vector<std::uint8_t>& payload, QString& message)
{
    message.resize(payload.size() - 2);

    memcpy(message.data(), payload.data() + 2, payload.size() - 2);
}

#endif // THORQ_PAYLOAD_EVENT_H
