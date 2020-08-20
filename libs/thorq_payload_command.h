#ifndef THORQ_PAYLOAD_COMMAND_H
#define THORQ_PAYLOAD_COMMAND_H

#include <string>
#include <vector>

#include "enums.h"

/** @typedef thorq_command_id_t
 */
typedef enum {
    THORQ_COMMAND_ID_LOGIN,
    THORQ_COMMAND_ID_LOGOUT,
    THORQ_COMMAND_ID_SET_SELF_STATE,
    THORQ_COMMAND_ID_GET_USER_LIST,
    THORQ_COMMAND_ID_SESSION_REQUEST,
    THORQ_COMMAND_ID_SESSION_ACCEPT,
    THORQ_COMMAND_ID_SESSION_DENY,
    THORQ_COMMAND_ID_SESSION_LEAVE,
} thorq_command_id_t;

/**
 * @brief thorq_message_command_is_valid
 * @param payload
 * @return
 */
inline bool thorq_message_command_is_valid(const std::vector<std::uint8_t>& payload)
{
    if (payload.size() >= 2 && payload[0] == THORQ_MESSAGE_ID_COMMAND)
    {
        switch (static_cast<thorq_command_id_t>(payload[1])) {
        case THORQ_COMMAND_ID_LOGOUT:
        case THORQ_COMMAND_ID_GET_USER_LIST:
        case THORQ_COMMAND_ID_SESSION_LEAVE:
            return payload.size() == 2;
        case THORQ_COMMAND_ID_SET_SELF_STATE:
            return payload.size() == 3;
        case THORQ_COMMAND_ID_LOGIN:
        case THORQ_COMMAND_ID_SESSION_REQUEST:
        case THORQ_COMMAND_ID_SESSION_ACCEPT:
        case THORQ_COMMAND_ID_SESSION_DENY:
            return payload.size() >= 3;
        default:
            return false;
        }
    }
    return false;
}

/**
 * @brief thorq_message_command_pack
 * @param payload
 * @param cmd_id
 */
inline void thorq_message_command_pack(std::vector<std::uint8_t>& payload, const thorq_command_id_t& cmd_id)
{
    payload.resize(2);
    payload[0] = THORQ_MESSAGE_ID_COMMAND;
    payload[1] = static_cast<std::uint8_t>(cmd_id);
}

/**
 * @brief thorq_message_command_pack
 * @param payload
 * @param cmd_id
 * @param data
 */
inline void thorq_message_command_pack(std::vector<std::uint8_t>& payload, const thorq_command_id_t& cmd_id, const std::string& data)
{
    payload.resize(2 + data.length());
    payload[0] = THORQ_MESSAGE_ID_COMMAND;
    payload[1] = static_cast<std::uint8_t>(cmd_id);

    memcpy(payload.data() + 2, data.data(), data.size());
}

/**
 * @brief thorq_message_command_pack
 * @param payload
 * @param cmd_id
 * @param data
 */
inline void thorq_message_command_pack(std::vector<std::uint8_t>& payload, const thorq_command_id_t& cmd_id, std::uint8_t data)
{
    payload.resize(3);
    payload[0] = THORQ_MESSAGE_ID_COMMAND;
    payload[1] = static_cast<std::uint8_t>(cmd_id);
    payload[2] = data;
}

/**
 * @brief thorq_message_command_get_id
 * @param payload
 * @param id
 */
inline void thorq_message_command_get_id(const std::vector<std::uint8_t>& payload, thorq_command_id_t& id)
{
    id = static_cast<thorq_command_id_t>(payload[1]);
}

/**
 * @brief thorq_message_command_get_data
 * @param payload
 * @param data
 */
inline void thorq_message_command_get_data(const std::vector<std::uint8_t>& payload, std::string& data)
{
    data.resize(payload.size() - 2);

    memcpy(data.data(), payload.data() + 2, payload.size() - 2);
}

/**
 * @brief thorq_message_command_get_data
 * @param payload
 * @param data
 */
inline void thorq_message_command_get_data(const std::vector<std::uint8_t>& payload, std::uint8_t& data)
{
    data = payload[2];
}

#endif // THORQ_PAYLOAD_COMMAND_H
