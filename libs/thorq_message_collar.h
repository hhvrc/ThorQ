#ifndef THORQ_MESSAGE_COLLAR_H
#define THORQ_MESSAGE_COLLAR_H

#include "thorq_message.h"

/**
 * @brief The thorq_collar_t struct
 */
THORQPACKED(
typedef struct __thorq_collar
{
	std::uint8_t flags; ///< thorq_collar_flag_t
	std::uint8_t shock_value;
	std::uint8_t vibration_value;
	std::uint8_t beep_value;
	std::uint8_t auto_value;

	bool operator == (const __thorq_collar& other) const;
	bool operator != (const __thorq_collar& other) const;
}) thorq_collar_t;

#endif // THORQ_MESSAGE_COLLAR_H
