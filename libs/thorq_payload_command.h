#ifndef THORQ_PAYLOAD_COMMAND_H
#define THORQ_PAYLOAD_COMMAND_H

#include <string>

#include "thorq_payload.h"

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

inline bool thorq_payload_command_is_valid(const thorq_payload_t& payload)
{
    return payload.id == THORQ_PAYLOAD_ID_COMMAND;
}

inline void thorq_payload_command_pack(thorq_payload_t& payload, const thorq_command_id_t& cmd_id)
{
    payload.id = THORQ_PAYLOAD_ID_COMMAND;
	payload.data.resize(1);
    payload.data[0] = cmd_id;
}
inline void thorq_payload_command_pack(thorq_payload_t& payload, const thorq_command_id_t& cmd_id, const std::uint8_t& data)
{
	payload.id = THORQ_PAYLOAD_ID_COMMAND;
	payload.data.resize(2);
    payload.data[0] = cmd_id;
    payload.data[1] = data;
}
inline void thorq_payload_command_pack(thorq_payload_t& payload, const thorq_command_id_t& cmd_id, const std::string& str)
{
	payload.id = THORQ_PAYLOAD_ID_COMMAND;
	payload.data.resize(1 + str.length());
    payload.data[0] = cmd_id;
	memcpy(&payload.data[1], str.data(), str.length());
}

inline thorq_command_id_t thorq_payload_command_get_id(const thorq_payload_t& payload)
{
	return (thorq_command_id_t)payload.data[0];
}

inline std::uint8_t thorq_payload_command_get_data(const thorq_payload_t& payload)
{
	return payload.data[1];
}

inline std::string thorq_payload_command_get_string(const thorq_payload_t& payload)
{
	return std::string(payload.data.begin() + 1, payload.data.end());
}

#endif // THORQ_PAYLOAD_COMMAND_H
