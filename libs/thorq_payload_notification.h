#ifndef THORQ_MSG_PAYLOAD_NOTIFICATION_H
#define THORQ_MSG_PAYLOAD_NOTIFICATION_H

#include <string>

#include "thorq_payload.h"

typedef enum {
    EVENT,
    ADMIN_ANNOUNCEMENT,
    SYSTEM_ANNOUNCEMENT,
} thorq_notification_type_t;

typedef enum {
    MAINT
} thorq_announcement_reason_t;

inline void thorq_notification_pack(thorq_payload_t& msg )
{

}

inline thorq_announcement_source_t thorq_msg_announcement_get_source(const thorq_payload_t& msg)
{

}
inline thorq_announcement_severity_t thorq_msg_announcement_get_severity(const thorq_payload_t& msg)
{

}
inline thorq_announcement_reason_t thorq_msg_announcement_get_reason(const thorq_payload_t& msg)
{

}
inline std::string thorq_msg_announcement_get_message(const thorq_payload_t& msg)
{

}

#endif // THORQ_PAYLOAD_NOTIFICATION_H
