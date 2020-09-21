#ifndef THORQ_PAYLOAD_ACCOUNT_H
#define THORQ_PAYLOAD_ACCOUNT_H

#include <QString>
#include <vector>

#include "enums.h"

/// @enum THORQ_PAYLOAD_ACCOUNT_REQ
enum THORQ_PAYLOAD_ACCOUNT_REQ
{
    THORQ_PAYLOAD_ACCOUNT_REQ_RESERVE_DISCORD,  ///< Try to reserve a discordID
    THORQ_PAYLOAD_ACCOUNT_REQ_RESERVE_USERNAME, ///< Try to reserve a username
    THORQ_PAYLOAD_ACCOUNT_REQ_REGISTER,         ///< Register an account
    THORQ_PAYLOAD_ACCOUNT_REQ_DELETE,           ///< Delete an account (requires password)
    THORQ_PAYLOAD_ACCOUNT_REQ_LOGIN,            ///< Log in with an option of getting a sessionkey
    THORQ_PAYLOAD_ACCOUNT_REQ_LOGOUT,           ///< Log out, removes any session-key connected to this hwid
    THORQ_PAYLOAD_ACCOUNT_REQ_SET_STAUS         ///< Set status of self
};

/// @enum THORQ_PAYLOAD_ACCOUNT_ACK
enum THORQ_PAYLOAD_ACCOUNT_ACK
{
    THORQ_PAYLOAD_ACCOUNT_ACK_OK,        ///< RESERVE_DISCORD/RESERVE_USERNAME/REGISTER/DELETE/LOGIN/LOGOUT/DISCONNECT/SET_STATUS
    THORQ_PAYLOAD_ACCOUNT_ACK_DENIED,    ///< RESERVE_DISCORD/RESERVE_USERNAME/REGISTER/DELETE/LOGIN/------/----------/SET_STATUS
    THORQ_PAYLOAD_ACCOUNT_ACK_UNCHANGED, ///< RESERVE_DISCORD/RESERVE_USERNAME/--------/------/LOGIN/LOGOUT/DISCONNECT/SET_STATUS
    THORQ_PAYLOAD_ACCOUNT_ACK_ERROR,     ///< ---------------/----------------/REGISTER/DELETE/LOGIN/LOGOUT/DISCONNECT/SET_STATUS
    THORQ_PAYLOAD_ACCOUNT_ACK_INVALID    ///< INVALID REQUEST
};

/**
 * @brief thorq_payload_account_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_account_is_valid(const std::vector<std::uint8_t>& payload)
{
    if (payload[0] != THORQ_PAYLOAD_ID_ACCOUNT) return false;

    if (payload.size() >= 2)
    {
        if (payload[1] == THORQ_PAYLOAD_ACCOUNT_REQ_RESERVE_DISCORD && payload.size() >= THORQ_DISCORDID_LEN_MIN + 2 && payload.size() <= THORQ_DISCORDID_LEN_MAX + 2)
        {
            return true;
        }

        if (payload.size() == 3 &&
           (payload[1] == THORQ_PAYLOAD_LOGIN))
    }
    if (payload[1] ==)

    if (payload.size() >= 2 && )
    {
        switch (static_cast<THORQ_PAYLOAD_ACCOUNT>(payload[1])) {
        case THORQ_PAYLOAD_ACCOUNT_REGISTER:
            return payload.size() == 2;
        case THORQ_PAYLOAD_ACCOUNT_DELETE:
            return payload.size() == 2;
        case THORQ_PAYLOAD_ACCOUNT_SET_STAUS:
            return payload.size() == 3;
        default:
            return payload.size() > 3
                && payload[1] == THORQ_PAYLOAD_ACCOUNT_LOGIN
                && payload[2] >= THORQ_USERNAME_LEN_MIN
                && payload[2] <= THORQ_USERNAME_LEN_MAX
                && payload[3] >= THORQ_PASSWORD_LEN_MIN
                && payload[3] <= THORQ_PASSWORD_LEN_MAX
                && payload.size() == 3 + payload[2] + payload[3];
        }
    }


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
