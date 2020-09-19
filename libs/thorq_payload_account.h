#ifndef THORQ_PAYLOAD_ACCOUNT_H
#define THORQ_PAYLOAD_ACCOUNT_H

#include <QString>
#include <vector>

#include "enums.h"

/// @enum THORQ_PAYLOAD_ACCOUNT
enum THORQ_PAYLOAD_ACCOUNT
{
    THORQ_PAYLOAD_ACCOUNT_REGISTER,   ///< Register an account
    THORQ_PAYLOAD_ACCOUNT_DELETE,     ///< Delete an account
    THORQ_PAYLOAD_ACCOUNT_LOGIN,      ///< Log in with an option of getting a sessionkey
    THORQ_PAYLOAD_ACCOUNT_LOGOUT,     ///< Log out, removes any session-key connected to this hwid
    THORQ_PAYLOAD_ACCOUNT_DISCONNECT, ///< Disconnect, doesnt remove session-key
    THORQ_PAYLOAD_ACCOUNT_SET_STAUS   ///<
};

/**
 * @brief thorq_payload_account_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_account_is_valid(const std::vector<std::uint8_t>& payload)
{
    if (payload.size() >= 2 && payload[0] == THORQ_PAYLOAD_ID_ACCOUNT)
    {
        switch (static_cast<THORQ_PAYLOAD_ACCOUNT>(payload[1])) {
        case THORQ_PAYLOAD_ACCOUNT_REGISTER:
            return payload.size() == 2;
        case THORQ_PAYLOAD_ACCOUNT_DELETE:
            return payload.size() == 2;
        case THORQ_PAYLOAD_ACCOUNT_SET_STAUS:
            return payload.size() == 3;
        default:
            return false;
        }
    }
    return false;
}

/**
 * @brief thorq_payload_account_pack
 * @param payload
 * @param cmd_id
 */
inline void thorq_payload_account_pack(std::vector<std::uint8_t>& payload, const THORQ_PAYLOAD_ACCOUNT& cmd_id)
{
    payload.resize(2);
    payload[0] = THORQ_PAYLOAD_ID_ACCOUNT;
    payload[1] = static_cast<std::uint8_t>(cmd_id);
}

/**
 * @brief thorq_payload_account_pack
 * @param payload
 * @param cmd_id
 * @param data
 */
inline void thorq_payload_account_pack(std::vector<std::uint8_t>& payload, const THORQ_PAYLOAD_ACCOUNT& cmd_id, std::uint8_t data)
{
    payload.resize(3);
    payload[0] = THORQ_PAYLOAD_ID_ACCOUNT;
    payload[1] = static_cast<std::uint8_t>(cmd_id);
    payload[2] = data;
}

/**
 * @brief thorq_payload_account_get_id
 * @param payload
 * @param id
 */
inline void thorq_payload_account_get_id(const std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_ACCOUNT& id)
{
    id = static_cast<THORQ_PAYLOAD_ACCOUNT>(payload[1]);
}

/**
 * @brief thorq_payload_account_get_data
 * @param payload
 * @param data
 */
inline void thorq_payload_account_get_data(const std::vector<std::uint8_t>& payload, std::uint8_t& data)
{
    data = payload[2];
}

#endif // THORQ_PAYLOAD_ACCOUNT_H
