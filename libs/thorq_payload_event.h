#ifndef THORQ_PAYLOAD_EVENT_H
#define THORQ_PAYLOAD_EVENT_H

#include <string>
#include "thorq_payload.h"

/** @file thorq_payload_event.h
 *
 */

/** @typedef thorq_event_type_t
 *
 */
typedef enum {
    THORQ_EVENT_SESSION_REQUESTED,
    THORQ_EVENT_SESSION_DENIED,
    THORQ_EVENT_SESSION_STARTED,
    THORQ_EVENT_SESSION_STOPPED,
} thorq_event_type_t;

/**
 * @brief thorq_payload_event_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_event_is_valid(const thorq_payload_t& payload)
{
    return payload.id == THORQ_PAYLOAD_ID_EVENT;
}

/**
 * @brief thorq_payload_event_pack
 * @param payload
 * @param type
 * @param data
 */
inline void thorq_payload_event_pack(thorq_payload_t& payload, const thorq_event_type_t& type, const std::string& data)
{
    payload.id = THORQ_PAYLOAD_ID_EVENT;
    payload.data.resize(1 + data.size());
    payload.data[0] = static_cast<std::uint8_t>(type);
    if (data.size() != 0)
        memcpy(&payload.data[1], &data[0], data.size());
}

/**
 * @brief thorq_payload_event_get_type
 * @param payload
 * @param type
 */
inline void thorq_payload_event_get_type(const thorq_payload_t& payload, thorq_event_type_t& type)
{
    type = static_cast<thorq_event_type_t>(payload.data[0]);
}

/**
 * @brief thorq_payload_event_get_message
 * @param payload
 * @param message
 */
inline void thorq_payload_event_get_message(const thorq_payload_t& payload, std::string& message)
{
    message.resize(payload.data.size() - 1);
    if (message.size() != 0)
        memcpy(&message[0], &payload.data[1], payload.data.size() - 1);
}

#endif // THORQ_PAYLOAD_EVENT_H
