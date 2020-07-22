#ifndef THORQ_MSG_ADMIN_BROADCAST_H
#define THORQ_MSG_ADMIN_BROADCAST_H

#include "thorq_message.h"

/**
 * @brief The thorq_announcement_t struct
 */
THORQPACKED(
typedef struct __thorq_announcement
{
	std::uint8_t source;
	std::uint8_t severity;
	std::uint8_t reason;
	char message[THORQ_MSG_MAX_PAYLOAD_LEN - 2];

	bool operator == (const __thorq_announcement& other) const;
	bool operator != (const __thorq_announcement& other) const;
}) thorq_announcement_t;

#endif // THORQ_MSG_ANNOUNCEMENT_H
