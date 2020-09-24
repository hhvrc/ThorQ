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
	THORQ_PAYLOAD_SYSTEMID_REQ,
	THORQ_PAYLOAD_SYSTEMID_DATA,
	THORQ_PAYLOAD_SYSTEMID_OK
};

inline bool thorq_payload_systemid_is_valid(const std::vector<std::uint8_t>& payload)
{
    if (payload[0] != THORQ_PAYLOAD_ID_SYSTEMID) return false;

    if (payload.size() == 2)
    {
		return payload[1] == THORQ_PAYLOAD_SYSTEMID_REQ
			|| payload[1] == THORQ_PAYLOAD_SYSTEMID_OK;
    }

    std::size_t dataSize = payload.size() - 2;

    return dataSize >= THORQ_AUTH_SYSTEMID_LEN_MIN
        && dataSize <= THORQ_AUTH_SYSTEMID_LEN_MAX
        && payload[1] == THORQ_PAYLOAD_SYSTEMID_DATA;
}

inline void thorq_payload_systemid_cmd_pack(std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_SYSTEMID cmd)
{
	payload.resize(2);
	payload[0] = THORQ_PAYLOAD_ID_SYSTEMID;
	payload[1] = static_cast<std::uint8_t>(cmd);
}
inline void thorq_payload_systemid_data_pack(std::vector<std::uint8_t>& payload, const QByteArray& systemid)
{
	payload.resize(2 + systemid.size());
	payload[0] = THORQ_PAYLOAD_ID_SYSTEMID;
	payload[1] = THORQ_PAYLOAD_SYSTEMID_DATA;

	memcpy(payload.data() + 2, systemid.data(), systemid.size());
}
inline void thorq_payload_systemid_get_cmd(const std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_SYSTEMID& cmd)
{
	cmd = static_cast<THORQ_PAYLOAD_SYSTEMID>(payload[1]);
}
inline void thorq_payload_systemid_get_data(const std::vector<std::uint8_t>& payload, QByteArray& systemid)
{
	systemid.resize(payload.size() - 2);
	memcpy(systemid.data(), payload.data() + 2, payload.size() - 2);
}

#endif // THORQ_PAYLOAD_SYSTEMID_H
