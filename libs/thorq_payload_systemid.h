/// @file thorq_payload_systemid.h
///
///

#ifndef THORQ_PAYLOAD_SYSTEMID_H
#define THORQ_PAYLOAD_SYSTEMID_H

#include <vector>
#include <cstdint>

#include <QByteArray>

#include "constants.h"
#include "enums.h"

/// @enum THORQ_PAYLOAD_REGKEY
enum THORQ_PAYLOAD_SYSTEMID : std::uint8_t
{
    THORQ_PAYLOAD_SYSTEMID_DATA,
    THORQ_PAYLOAD_SYSTEMID_CMD_REQ,
    THORQ_PAYLOAD_SYSTEMID_CMD_OK
};

inline bool thorq_payload_systemid_is_valid(const std::vector<std::uint8_t>& payload)
{
    if (payload[0] != THORQ_PAYLOAD_ID_SYSTEMID) return false;

    if (payload.size() == 2)
    {
        return payload[1] == THORQ_PAYLOAD_SYSTEMID_CMD_REQ
            || payload[1] == THORQ_PAYLOAD_SYSTEMID_CMD_OK;
    }

    std::size_t dataSize = payload.size() - 2;

    return dataSize >= THORQ_AUTH_SYSTEMID_LEN_MIN
        && dataSize <= THORQ_AUTH_SYSTEMID_LEN_MAX
        && payload[1] == THORQ_PAYLOAD_SYSTEMID_DATA;
}

inline void thorq_payload_systemid_pack(std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_SYSTEMID cmd)
{

}

#endif // THORQ_PAYLOAD_SYSTEMID_H
