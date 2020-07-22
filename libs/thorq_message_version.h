#ifndef THORQ_MSG_VERSION_H
#define THORQ_MSG_VERSION_H

#include <string>

#include "thorq_message.h"

/**
 * @brief The thorq_version_t struct
 */
THORQPACKED(
typedef struct __thorq_version
{
	std::uint8_t major = 0;
	std::uint8_t minor = 0;
	std::uint8_t patch = 0;

	bool isCurrentClientVersion() const;
	bool isCurrentServerVersion() const;
	bool isCurrentLinkVersion() const;

	void toString(char* c_str) const;
	void toString(std::string& str) const;

	bool operator == (const __thorq_version& other) const;
	bool operator != (const __thorq_version& other) const;
}) thorq_version_t;

#endif // THORQ_MSG_VERSION_H
