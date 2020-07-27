#ifndef THORQ_PAYLOAD_ACKNOWLEDGE_H
#define THORQ_PAYLOAD_ACKNOWLEDGE_H

#include "thorq_message.h"

typedef enum {
	OK,
	INFO,
	WARNING,
	ERROR,
	FATAL
} thorq_acknowledge_error_t;

inline void thorq_payload_acknowledge_pack()
{

}
inline void thorq_payload_acknowledge_get_error()
{

}

#endif // THORQ_PAYLOAD_ACKNOWLEDGE_H
