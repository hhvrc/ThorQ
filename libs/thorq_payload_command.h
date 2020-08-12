#ifndef THORQ_PAYLOAD_COMMAND_H
#define THORQ_PAYLOAD_COMMAND_H

#include <string>

#include "thorq_payload.h"

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
 * @brief thorq_payload_command_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_command_is_valid(const thorq_payload_t& payload)
{
    switch (static_cast<thorq_command_id_t>(payload.data[0])) {
    case THORQ_COMMAND_ID_LOGIN:
		return payload.data.size() >= 2;
    case THORQ_COMMAND_ID_LOGOUT:
        return payload.data.size() == 1;
    case THORQ_COMMAND_ID_SET_SELF_STATE:
        return payload.data.size() == 2;
    case THORQ_COMMAND_ID_GET_USER_LIST:
        return payload.data.size() == 1;
    case THORQ_COMMAND_ID_SESSION_REQUEST:
		return payload.data.size() >= 2;
    case THORQ_COMMAND_ID_SESSION_ACCEPT:
		return payload.data.size() >= 2;
    case THORQ_COMMAND_ID_SESSION_DENY:
		return payload.data.size() >= 2;
    case THORQ_COMMAND_ID_SESSION_LEAVE:
        return payload.data.size() == 1;
    }
}

/**
 * @brief thorq_payload_command_pack
 * @param payload
 * @param cmd_id
 */
inline void thorq_payload_command_pack(thorq_payload_t& payload, const thorq_command_id_t& cmd_id)
{
    payload.id = THORQ_PAYLOAD_ID_COMMAND;
    payload.data.resize(1);
    payload.data[0] = static_cast<std::uint8_t>(cmd_id);
}

/**
 * @brief thorq_payload_command_pack
 * @param payload
 * @param cmd_id
 * @param data
 */
inline void thorq_payload_command_pack(thorq_payload_t& payload, const thorq_command_id_t& cmd_id, const std::string& data)
{
	payload.id = THORQ_PAYLOAD_ID_COMMAND;
    payload.data.resize(1 + data.length());
    payload.data[0] = static_cast<std::uint8_t>(cmd_id);
    if (data.size() != 0)
        memcpy(&payload.data[1], &data[0], data.size());
}

/**
 * @brief thorq_payload_command_pack
 * @param payload
 * @param cmd_id
 * @param data
 */
inline void thorq_payload_command_pack(thorq_payload_t& payload, const thorq_command_id_t& cmd_id, std::uint8_t data)
{
    payload.id = THORQ_PAYLOAD_ID_COMMAND;
    payload.data.resize(2);
    payload.data[0] = static_cast<std::uint8_t>(cmd_id);
    payload.data[0] = data;
}

/**
 * @brief thorq_payload_command_get_id
 * @param payload
 * @param id
 */
inline void thorq_payload_command_get_id(const thorq_payload_t& payload, thorq_command_id_t& id)
{
    id = static_cast<thorq_command_id_t>(payload.data[0]);
}

/**
 * @brief thorq_payload_command_get_data
 * @param payload
 * @param data
 */
inline void thorq_payload_command_get_data(const thorq_payload_t& payload, std::string& data)
{
    data.resize(payload.data.size() - 1);
    if (data.size() != 0)
        memcpy(&data[0], &payload.data[1], payload.data.size() - 1);
}

/**
 * @brief thorq_payload_command_get_data
 * @param payload
 * @param data
 */
inline void thorq_payload_command_get_data(const thorq_payload_t& payload, std::uint8_t& data)
{
    data = payload.data[1];
}

#endif // THORQ_PAYLOAD_COMMAND_H
