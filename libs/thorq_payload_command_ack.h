#ifndef THORQ_MESSAGE_COMMAND_ACK_H
#define THORQ_MESSAGE_COMMAND_ACK_H

#include "thorq_payload_command.h"

inline bool thorq_payload_command_ack_is_valid(const thorq_payload_t& payload)
{
	return payload.id == THORQ_PAYLOAD_ID_COMMAND_ACK;
}

typedef enum {
	THORQ_COMMAND_ACK_OK,
	THORQ_COMMAND_ACK_
};

#endif // THORQ_MESSAGE_COMMAND_ACK_H
