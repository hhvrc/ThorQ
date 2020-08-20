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
 * @brief thorq_message_command_ack_is_valid
 * @param payload
 * @return
 */
inline bool thorq_message_command_ack_is_valid(const std::vector<std::uint8_t>& payload)
{
    return payload.size() >= 3 && payload[0] == THORQ_MESSAGE_ID_COMMAND_ACK;
}

/**
 * @brief thorq_message_command_ack_pack
 * @param payload
 * @param cmd
 * @param result
 */
inline void thorq_message_command_ack_pack(std::vector<std::uint8_t>& payload, thorq_command_id_t cmd, thorq_command_ack_result_t result)
{
    payload.resize(3);
    payload[0] = THORQ_MESSAGE_ID_COMMAND_ACK;
    payload[1] = static_cast<std::uint8_t>(cmd);
    payload[2] = static_cast<std::uint8_t>(result);
}

/**
 * @brief thorq_message_command_ack_pack
 * @param payload
 * @param cmd
 * @param result
 * @param string
 */
inline void thorq_message_command_ack_pack(std::vector<std::uint8_t>& payload, thorq_command_id_t cmd, thorq_command_ack_result_t result, const std::string& string)
{
    payload.resize(3 + string.length());
    payload[0] = THORQ_MESSAGE_ID_COMMAND_ACK;
    payload[1] = static_cast<std::uint8_t>(cmd);
    payload[2] = static_cast<std::uint8_t>(result);

    memcpy(payload.data() + 3, string.data(), string.size());
}

/**
 * @brief thorq_message_command_ack_get_cmd
 * @param payload
 * @param cmd
 */
inline void thorq_message_command_ack_get_cmd(const std::vector<std::uint8_t>& payload, thorq_command_id_t& cmd)
{
    cmd = static_cast<thorq_command_id_t>(payload[1]);
}

/**
 * @brief thorq_message_command_ack_get_result
 * @param payload
 * @param result
 */
inline void thorq_message_command_ack_get_result(const std::vector<std::uint8_t>& payload, thorq_command_ack_result_t& result)
{
    result = static_cast<thorq_command_ack_result_t>(payload[2]);
}

/**
 * @brief thorq_message_command_ack_get_message
 * @param payload
 * @param string
 */
inline void thorq_message_command_ack_get_message(const std::vector<std::uint8_t>& payload, std::string& string)
{
    string.resize(payload.size() - 3);

    memcpy(string.data(), payload.data() + 3, payload.size() - 3);
}

#endif // THORQ_MESSAGE_COMMAND_ACK_H
