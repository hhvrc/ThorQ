#ifndef THORQ_MSG_PAYLOAD_ANNOUNCEMENT_H
#define THORQ_MSG_PAYLOAD_ANNOUNCEMENT_H

#include "thorq_payload.h"

typedef enum {
	SYSTEM,
	ADMIN
} thorq_announcement_source_t;

typedef enum {

} thorq_announcement_severity_t;

typedef enum {

} thorq_announcement_reason_t;

inline thorq_announcement_source_t thorq_msg_announcement_get_source(const thorq_payload_t& msg);
inline thorq_announcement_severity_t thorq_msg_announcement_get_severity(const thorq_payload_t& msg);
inline thorq_announcement_reason_t thorq_msg_announcement_get_reason(const thorq_payload_t& msg);
inline std::string  thorq_msg_announcement_get_message(const thorq_payload_t& msg);

#endif // THORQ_PAYLOAD_ANNOUNCEMENT_H
