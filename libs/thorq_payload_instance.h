#ifndef THORQ_PAYLOAD_INSTANCE_H
#define THORQ_PAYLOAD_INSTANCE_H

#include <vector>
#include <algorithm>

#include <QString>
#include <QtGlobal>

#include "enums.h"
#include "constants.h"

enum THORQ_PAYLOAD_INSTANCE
{
};

/**
 * @brief thorq_payload_login_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_instance_is_valid(const std::vector<std::uint8_t>& payload)
{
    if (payload[0] != THORQ_PAYLOAD_ID_INSTANCE)

    return payload.size() > 3
        && payload[0] == THORQ_PAYLOAD_ID_LOGIN
        && payload[1] >= THORQ_USERNAME_LEN_MIN
        && payload[1] <= THORQ_USERNAME_LEN_MAX
        && payload[2] >= THORQ_PASSWORD_LEN_MIN
        && payload[2] <= THORQ_PASSWORD_LEN_MAX
        && payload.size() == 3 + payload[1] + payload[2];
}

#endif // THORQ_PAYLOAD_INSTANCE_H
