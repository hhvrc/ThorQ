/// @file thorq_payload_session.h
///
///

#ifndef THORQ_PAYLOAD_SESSION_H
#define THORQ_PAYLOAD_SESSION_H

#include <vector>
#include <string>
#include <cstdint>

#include "enums.h"

/// @enum THORQ_PAYLOAD_SESSION
enum THORQ_PAYLOAD_SESSION : std::uint8_t
{
    THORQ_PAYLOAD_SESSION_REQUEST,
    THORQ_PAYLOAD_SESSION_ACCEPT,
    THORQ_PAYLOAD_SESSION_DENY,
    THORQ_PAYLOAD_SESSION_LEAVE
};

inline bool thorq_payload_session_is_valid(const std::vector<std::uint8_t>& payload)
{
    if (payload[0] != THORQ_PAYLOAD_ID_SESSION) return false;

    if (payload.size() == 2)
    {
        return payload[1] == THORQ_PAYLOAD_SESSION_ACCEPT
            || payload[1] == THORQ_PAYLOAD_SESSION_DENY
            || payload[1] == THORQ_PAYLOAD_SESSION_LEAVE;
    }

    return payload.size() > 2 && payload[1] == THORQ_PAYLOAD_SESSION_REQUEST;
}

inline void thorq_payload_session_pack(std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_SESSION cmd, const std::string& username)
{
    payload.resize(2 + username.size());
    payload[0] = THORQ_PAYLOAD_ID_SESSION;
    payload[1] = THORQ_PAYLOAD_SESSION_REQUEST;
    memcpy(payload.data() + 2, username.data(), username.size());
}

inline void thorq_payload_session_get_cmd(const std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_SESSION& cmd)
{
    cmd = static_cast<THORQ_PAYLOAD_SESSION>(payload[0]);
}

inline void thorq_payload_session_get_data(const std::vector<std::uint8_t>& payload, std::string& username)
{
    username = std::string((char*)payload.data() + 2, payload.size() - 2);
}

#endif // THORQ_PAYLOAD_SESSION_H
