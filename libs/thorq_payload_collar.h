#ifndef THORQ_PAYLOAD_COLLAR_H
#define THORQ_PAYLOAD_COLLAR_H

#include "thorq_payload.h"

inline std::uint8_t thorq_payload_collar_is_valid(const thorq_payload_t* payload)
{

}

inline void thorq_payload_collar_pack(thorq_payload_t& payload, const std::uint8_t& flags, const std::uint8_t& shock_value, const std::uint8_t& vibration_value, const std::uint8_t& beep_value, const std::uint8_t& auto_value)
{
	payload.id = THORQ_PAYLOAD_ID_COLLAR;
	payload.size = 5;
	payload.data[0] = flags;
	payload.data[1] = shock_value;
	payload.data[2] = vibration_value;
	payload.data[3] = beep_value;
	payload.data[4] = auto_value;
}
inline void thorq_payload_collar_unpack(const thorq_payload_t& payload, std::uint8_t& flags, std::uint8_t& shock_value, std::uint8_t& vibration_value, std::uint8_t& beep_value, std::uint8_t& auto_value)
{

}

#endif // THORQ_PAYLOAD_COLLAR_H
