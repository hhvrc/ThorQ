#ifndef THORQ_PAYLOAD_RELATIONSHIP_H
#define THORQ_PAYLOAD_RELATIONSHIP_H

#include <cstdint>
#include <vector>
#include <string>

#include "enums.h"
#include "serialization.h"

/// @enum THORQ_PAYLOAD_FRIEND
enum class THORQ_PAYLOAD_RELATIONSHIP : std::uint8_t
{
    THORQ_PAYLOAD_RELATIONSHIP_BLOCK_APPLY,
    THORQ_PAYLOAD_RELATIONSHIP_BLOCK_REMOVE,
    THORQ_PAYLOAD_RELATIONSHIP_BLOCK_LIST,

    THORQ_PAYLOAD_RELATIONSHIP_FRIEND_REQUEST,
    THORQ_PAYLOAD_RELATIONSHIP_FRIEND_ACCEPT,
    THORQ_PAYLOAD_RELATIONSHIP_FRIEND_DENY,
    THORQ_PAYLOAD_RELATIONSHIP_FRIEND_LIST,

    THORQ_PAYLOAD_RELATIONSHIP_FRIEND_ADDED,  ///< Friend removed
    THORQ_PAYLOAD_RELATIONSHIP_FRIEND_STATUS, ///< Online/Offline/Session
    THORQ_PAYLOAD_RELATIONSHIP_FRIEND_REMOVED ///< Friend removed
};

inline bool thorq_payload_relationship_is_valid(const std::vector<std::uint8_t>& payload)
{
    if (thorq_payload_serialization_get_id(payload) != THORQ_PAYLOAD_ID_ACCOUNT) return false;

    if (thorq_payload_serialization_is_valid(payload))
    {
        switch (thorq_payload_serialization_get_cmd(payload)) {
        case THORQ_PAYLOAD_ACCOUNT_REGISTER:
            return payload.size() > 5
                    && payload[2] >= THORQ_USERNAME_LEN_MIN
                    && payload[2] <= THORQ_USERNAME_LEN_MAX
                    && payload[3] >= THORQ_PASSWORD_LEN_MIN
                    && payload[3] <= THORQ_PASSWORD_LEN_MAX
                    && payload[4] >= THORQ_DISCORDID_LEN_MIN
                    && payload[4] <= THORQ_DISCORDID_LEN_MAX
                    && payload.size() == 5 + payload[2] + payload[3] + payload[4];
        case THORQ_PAYLOAD_ACCOUNT_DELETE:
            return payload.size() > 3
                    && payload[1] >= THORQ_PASSWORD_LEN_MIN
                    && payload[1] <= THORQ_PASSWORD_LEN_MAX
                    && payload.size() == 3 + payload[2];
        case THORQ_PAYLOAD_ACCOUNT_LOGIN:
            return payload.size() > 4
                    && payload[2] >= THORQ_USERNAME_LEN_MIN
                    && payload[2] <= THORQ_USERNAME_LEN_MAX
                    && payload[3] >= THORQ_PASSWORD_LEN_MIN
                    && payload[3] <= THORQ_PASSWORD_LEN_MAX
                    && payload.size() == 4 + payload[2] + payload[3];
        case THORQ_PAYLOAD_ACCOUNT_LOGIN_AUTHTOKEN:
            return payload.size() == THORQ_AUTHTOKEN_LEN + 2;
        case THORQ_PAYLOAD_ACCOUNT_LOGOUT:
            return payload.size() == 3 && payload[2] <= THORQ_PAYLOAD_ACCOUNT_LOGOUT_OPTIONS_ALL;
        default:
            return false;
        }
    }

    return false;
}

inline THORQ_PAYLOAD_ACCOUNT thorq_payload_relationship_get_cmd(const std::vector<std::uint8_t>& payload)
{
    return static_cast<THORQ_PAYLOAD_ACCOUNT>(thorq_payload_serialization_get_cmd(payload));
}

inline void thorq_payload_relationship_register_pack(std::vector<std::uint8_t>& payload, const std::string& username, const std::string& password)
{
    thorq_payload_serialization_pack_string(payload, THORQ_PAYLOAD_ID_ACCOUNT, THORQ_PAYLOAD_ACCOUNT_REGISTER, username, password);
}
inline void thorq_payload_relationship_register_unpack(const std::vector<std::uint8_t>& payload, std::string& username, std::string& password)
{
    thorq_payload_serialization_unpack_string(payload, username, password);
}

inline void thorq_payload_relationship_delete_pack(std::vector<std::uint8_t>& payload, const std::string& password)
{
    thorq_payload_serialization_pack_string(payload, THORQ_PAYLOAD_ID_ACCOUNT, THORQ_PAYLOAD_ACCOUNT_DELETE, std::string(password, THORQ_PASSWORD_LEN_MAX));
}
inline void thorq_payload_relationship_delete_unpack(const std::vector<std::uint8_t>& payload, std::string& password)
{
    thorq_payload_serialization_unpack_string(payload, password);
}

inline void thorq_payload_relationship_login_pack(std::vector<std::uint8_t>& payload, const std::string& username, const std::string& password)
{
    thorq_payload_serialization_pack_string(payload, THORQ_PAYLOAD_ID_ACCOUNT, THORQ_PAYLOAD_ACCOUNT_LOGIN, std::string(username, THORQ_USERNAME_LEN_MAX), std::string(password, THORQ_PASSWORD_LEN_MAX));
}
inline void thorq_payload_relationship_login_unpack(const std::vector<std::uint8_t>& payload, std::string& username, std::string& password)
{
    thorq_payload_serialization_unpack_string(payload, username, password);
}

inline void thorq_payload_relationship_login_authtoken_pack(std::vector<std::uint8_t>& payload, const std::string& username, const std::string& authtoken)
{
    thorq_payload_serialization_pack_string(payload, THORQ_PAYLOAD_ID_ACCOUNT, THORQ_PAYLOAD_ACCOUNT_LOGIN_AUTHTOKEN, username, authtoken);
}
inline void thorq_payload_account_login_authtoken_unpack(const std::vector<std::uint8_t>& payload, std::string& username, std::string& authtoken)
{
    thorq_payload_serialization_unpack_string(payload, username, authtoken);
}

inline void thorq_payload_account_logout_pack(std::vector<std::uint8_t>& payload, std::uint8_t options = THORQ_PAYLOAD_ACCOUNT_LOGOUT_OPTIONS_NONE)
{
    thorq_payload_serialization_bytes_pack(payload, THORQ_PAYLOAD_ID_ACCOUNT, THORQ_PAYLOAD_ACCOUNT_LOGOUT, { options });
}
inline void thorq_payload_account_logout_unpack(const std::vector<std::uint8_t>& payload, std::uint8_t& options)
{
    options = thorq_payload_serialization_bytes_get(payload, 0);
}

#endif // THORQ_PAYLOAD_RELATIONSHIP_H
