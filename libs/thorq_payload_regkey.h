/// @file thorq_payload_regkey.h
///
///

#ifndef THORQ_PAYLOAD_REGKEY_H
#define THORQ_PAYLOAD_REGKEY_H

#include <vector>
#include <cstdint>

#include <QString>

#include "constants.h"
#include "enums.h"

/// @enum THORQ_PAYLOAD_REGKEY
enum THORQ_PAYLOAD_REGKEY : std::uint8_t
{
    THORQ_PAYLOAD_REGKEY_REQ,
    THORQ_PAYLOAD_REGKEY_AWAITING_INPUT,
	THORQ_PAYLOAD_REGKEY_DATA,
    THORQ_PAYLOAD_REGKEY_OK
};

inline bool thorq_payload_regkey_is_valid(const std::vector<std::uint8_t>& payload)
{
    if (payload[0] != THORQ_PAYLOAD_ID_REGKEY) return false;

	if (payload.size() == 2)
	{
		return payload[1] == THORQ_PAYLOAD_REGKEY_REQ
			|| payload[1] == THORQ_PAYLOAD_REGKEY_AWAITING_INPUT
			|| payload[1] == THORQ_PAYLOAD_REGKEY_OK;
	}

	std::size_t dataSize = payload.size() - 2;

	return dataSize >= THORQ_AUTH_REGKEY_LEN && payload[1] == THORQ_PAYLOAD_REGKEY_DATA;
}

inline void thorq_payload_regkey_get_id(const std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_REGKEY& id)
{
	id = static_cast<THORQ_PAYLOAD_REGKEY>(payload[1]);
}

inline void thorq_payload_regkey_pack(std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_REGKEY id)
{
    payload.resize(2);

    payload[0] = THORQ_PAYLOAD_ID_REGKEY;
	payload[1] = static_cast<std::uint8_t>(id);
}

inline void thorq_payload_regkey_data_pack(std::vector<std::uint8_t>& payload, const QString& key)
{
    QByteArray keyBytes = key.toUtf8();

    payload.resize(2 + keyBytes.size());

    payload[0] = THORQ_PAYLOAD_ID_REGKEY;
	payload[1] = THORQ_PAYLOAD_REGKEY_DATA;

    memcpy(payload.data() + 2, keyBytes.data(), keyBytes.size());
}
inline void thorq_payload_regkey_data_unpack(const std::vector<std::uint8_t>& payload, QString& key)
{
    key = QString::fromUtf8((char*)payload.data() + 2, payload.size() - 2);
}

#endif // THORQ_PAYLOAD_REGKEY_H
