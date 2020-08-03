#ifndef THORQ_PAYLOAD_EVENT_H
#define THORQ_PAYLOAD_EVENT_H

#include <string>
#include "thorq_payload.h"

typedef enum {
    THORQ_EVENT_USER_STATUS,
    THORQ_EVENT_USER_OFFLINE,
    THORQ_EVENT_USER_OFFLINE_LOS,
    THORQ_EVENT_USER_OFFLINE_TIMEOUT,
} thorq_event_type_t;

inline bool thorq_payload_event_is_valid(const thorq_payload_t& payload)
{
    return payload.id == THORQ_PAYLOAD_ID_EVENT;
}

inline void thorq_payload_event_pack(thorq_payload_t& payload, const thorq_event_type_t& type, const std::string& data)
{
    payload.id = THORQ_PAYLOAD_ID_EVENT;
    payload.data.resize(1 + data.size());
    payload.data[0] = static_cast<std::uint8_t>(type);
    memcpy(&payload.data[1], &data[0], data.size());
}

inline void thorq_payload_event_get_type(const thorq_payload_t& payload, thorq_event_type_t& type)
{
    type = static_cast<thorq_event_type_t>(payload.data[0]);
}

inline void thorq_payload_event_get_message(const thorq_payload_t& payload, std::string& message)
{
    message.resize(payload.data.size() - 1);
    memcpy(&message[0], &payload.data[1], payload.data.size() - 1);
}

#endif // THORQ_PAYLOAD_EVENT_H
