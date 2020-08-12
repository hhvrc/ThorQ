#ifndef THORQ_MESSAGE_COMMAND_ACK_H
#define THORQ_MESSAGE_COMMAND_ACK_H

#include "thorq_payload_command.h"

typedef enum {
    THORQ_COMMAND_ACK_RESULT_OK, ///< Command succeeded
    THORQ_COMMAND_ACK_RESULT_IN_PROGRESS, ///< Command accepted, and is in progress
    THORQ_COMMAND_ACK_RESULT_NO_CHANGE, ///< Command was ignored, because it didnt change anything

    THORQ_COMMAND_ACK_RESULT_DENIED, ///< Command was denied
    THORQ_COMMAND_ACK_RESULT_INVALID, ///< Command invalid
    THORQ_COMMAND_ACK_RESULT_LOGIN_NEEDED, ///< Client has not logged in
    THORQ_COMMAND_ACK_RESULT_UNAUTHORIZED, ///< Client has not authenticated (Crypto + Auth)
} thorq_command_ack_result_t; ///< Acknowledge of command sent from client

/**
 * @brief thorq_payload_command_ack_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_command_ack_is_valid(const thorq_payload_t& payload)
{
	return payload.id == THORQ_PAYLOAD_ID_COMMAND_ACK;
}

/**
 * @brief thorq_payload_command_ack_pack
 * @param payload
 * @param cmd
 * @param result
 */
inline void thorq_payload_command_ack_pack(thorq_payload_t& payload, thorq_command_id_t cmd, thorq_command_ack_result_t result)
{
    payload.id = THORQ_PAYLOAD_ID_COMMAND_ACK;
    payload.data.resize(2);
    payload.data[0] = static_cast<std::uint8_t>(cmd);
    payload.data[1] = static_cast<std::uint8_t>(result);
}

/**
 * @brief thorq_payload_command_ack_pack
 * @param payload
 * @param cmd
 * @param result
 * @param string
 */
inline void thorq_payload_command_ack_pack(thorq_payload_t& payload, thorq_command_id_t cmd, thorq_command_ack_result_t result, const std::string& string)
{
    payload.id = THORQ_PAYLOAD_ID_COMMAND_ACK;
    payload.data.resize(2 + string.length());
    payload.data[0] = static_cast<std::uint8_t>(cmd);
    payload.data[1] = static_cast<std::uint8_t>(result);
    if (string.size() != 0)
        memcpy(&payload.data[2], &string[0], string.size());
}

/**
 * @brief thorq_payload_command_ack_get_cmd
 * @param payload
 * @param cmd
 */
inline void thorq_payload_command_ack_get_cmd(const thorq_payload_t& payload, thorq_command_id_t& cmd)
{
    cmd = static_cast<thorq_command_id_t>(payload.data[0]);
}

/**
 * @brief thorq_payload_command_ack_get_result
 * @param payload
 * @param result
 */
inline void thorq_payload_command_ack_get_result(const thorq_payload_t& payload, thorq_command_ack_result_t& result)
{
    result = static_cast<thorq_command_ack_result_t>(payload.data[1]);
}

/**
 * @brief thorq_payload_command_ack_get_message
 * @param payload
 * @param string
 */
inline void thorq_payload_command_ack_get_message(const thorq_payload_t& payload, std::string& string)
{
    string.resize(payload.data.size() - 2);
    if (string.size() != 0)
        memcpy(&string[0], &payload.data[2], payload.data.size() - 2);
}

#endif // THORQ_MESSAGE_COMMAND_ACK_H
