#ifndef THORQ_PAYLOAD_COLLAR_H
#define THORQ_PAYLOAD_COLLAR_H

#include "thorq_payload.h"

typedef enum {
    THORQ_COLLAR_FLAG_SHOCK      = 1 << 0, ///< Activate collar shock
    THORQ_COLLAR_FLAG_VIBRATE    = 1 << 1, ///< Activate collar vibration
    THORQ_COLLAR_FLAG_BEEP       = 1 << 2, ///< Activate collar speaker
    THORQ_COLLAR_FLAG_AUTO       = 1 << 3, ///< Auto mode
    THORQ_COLLAR_FLAG_RESERVED_5 = 1 << 4,
    THORQ_COLLAR_FLAG_RESERVED_6 = 1 << 5,
    THORQ_COLLAR_FLAG_RESERVED_7 = 1 << 6,
    THORQ_COLLAR_FLAG_RESERVED_8 = 1 << 7,
} thorq_collar_flag_t; ///< Collar flag to describe current user input

inline bool thorq_payload_collar_is_valid(const thorq_payload_t& payload)
{
	return payload.id == THORQ_PAYLOAD_ID_COLLAR && payload.data.size() == 5;
}

inline void thorq_payload_collar_pack(thorq_payload_t& payload, const std::uint8_t& flags, const std::uint8_t& shock_value, const std::uint8_t& vibration_value, const std::uint8_t& beep_value, const std::uint8_t& auto_value)
{
	payload.id = THORQ_PAYLOAD_ID_COLLAR;
	payload.data.resize(5);
	payload.data[0] = flags;
	payload.data[1] = shock_value;
	payload.data[2] = vibration_value;
	payload.data[3] = beep_value;
	payload.data[4] = auto_value;
}

inline void thorq_payload_collar_unpack(const thorq_payload_t& payload, std::uint8_t& flags, std::uint8_t& shock_value, std::uint8_t& vibration_value, std::uint8_t& beep_value, std::uint8_t& auto_value)
{
    flags = payload.data[0];
    shock_value = payload.data[1];
    vibration_value = payload.data[2];
    beep_value = payload.data[3];
    auto_value = payload.data[4];
}

#endif // THORQ_PAYLOAD_COLLAR_H
