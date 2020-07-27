#ifndef THORQ_PAYLOAD_HEARTBEAT_H
#define THORQ_PAYLOAD_HEARTBEAT_H

#include "thorq_payload.h"

inline bool thorq_payload_heartbeat_is_valid(const thorq_payload_t& payload)
{
	return payload.id == THORQ_PAYLOAD_ID_HEARTBEAT && payload.data.empty();
}

inline void thorq_payload_heartbeat_pack(thorq_payload_t& payload)
{
	payload.id = THORQ_PAYLOAD_ID_HEARTBEAT;
	payload.data.clear();
}

#endif // THORQ_PAYLOAD_HEARTBEAT_H
