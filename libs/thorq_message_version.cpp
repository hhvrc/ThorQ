#include "thorq_message_version.h"

bool __thorq_version::isCurrentClientVersion() const
{
	return *this == thorq_msg_version_current_client;
}

bool __thorq_version::isCurrentServerVersion() const
{
	return *this == thorq_msg_version_current_server;
}

bool __thorq_version::isCurrentLinkVersion() const
{
	return *this == thorq_msg_version_current_link;
}

void __thorq_version::toString(char* c_str) const
{
	sprintf(c_str, "%u.%u.%u", major, minor, patch);
}

void __thorq_version::toString(std::string& str) const
{
	char buffer[16];
	snprintf(buffer, 16, "%u.%u.%u", major, minor, patch);
	str = buffer;
}

bool __thorq_version::operator ==(const __thorq_version& other) const
{
	return memcmp(this, &other, sizeof(__thorq_version)) == 0;
}

bool __thorq_version::operator !=(const __thorq_version& other) const
{
	return memcmp(this, &other, sizeof(__thorq_version)) != 0;
}
