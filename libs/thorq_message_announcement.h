#ifndef THORQ_MSG_MESSAGE_ANNOUNCEMENT_H
#define THORQ_MSG_MESSAGE_ANNOUNCEMENT_H

#include "thorq_message.h"

/**
 * @brief The thorq_announcement_t struct
 *
 * Is never encrypted
 * Get sent to all clients, no matter what state they are in
 *
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

static std::uint8_t thorq_msg_announcement_get_source(const thorq_msg_t& msg);
static std::uint8_t thorq_msg_announcement_get_severity(const thorq_msg_t& msg);
static std::uint8_t thorq_msg_announcement_get_reason(const thorq_msg_t& msg);
static std::string  thorq_msg_announcement_get_message(const thorq_msg_t& msg);
static void thorq_msg_announcement_encode(const thorq_announcement_t& announcement, thorq_msg_t& msg);
static void thorq_msg_announcement_decode(const thorq_msg_t& msg, thorq_announcement_t& announcement);

#endif // THORQ_MESSAGE_ANNOUNCEMENT_H
