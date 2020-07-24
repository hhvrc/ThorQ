#ifndef THORQ_PAYLOAD_VERSION_H
#define THORQ_PAYLOAD_VERSION_H

#include "thorq_payload.h"

#include <string>

typedef enum {
	THORQ_CMD_VERSION_SERVER,
	THORQ_CMD_VERSION_CLIENT,
	THORQ_CMD_VERSION_LINK,
} thorq_version_app_t;

inline bool thorq_payload_version_is_valid(const thorq_payload_t& payload)
{
	return payload.id == THORQ_PAYLOAD_ID_VERSION && payload.data_len == 4 &&
		(payload.data[0] == THORQ_CMD_VERSION_SERVER || payload.data[0] == THORQ_CMD_VERSION_CLIENT || payload.data[0] == THORQ_CMD_VERSION_LINK);
}

inline void thorq_payload_version_pack(thorq_payload_t& payload, const std::uint8_t& app, const std::uint8_t& major, const std::uint8_t& minor, const std::uint8_t& patch)
{
	payload.id = THORQ_PAYLOAD_ID_VERSION;
	payload.data_len = 4;
	payload.data[0] = app;
	payload.data[1] = major;
	payload.data[2] = minor;
	payload.data[3] = patch;
}
inline void thorq_payload_version_unpack(const thorq_payload_t& payload, std::uint8_t& app, std::uint8_t& major, std::uint8_t& minor, std::uint8_t& patch)
{
	app = payload.data[0];
	major = payload.data[1];
	minor = payload.data[2];
	patch = payload.data[3];
}

inline bool thorq_payload_version_is_current(const thorq_payload_t& payload)
{
	std::uint8_t ver[4]{0};

	thorq_payload_version_unpack(payload, ver[0], ver[1], ver[2], ver[3]);

	switch (ver[0]) {
	case THORQ_CMD_VERSION_SERVER:
		return ver[1] == THORQ_VERSION_SERVER_MAJOR && ver[2] == THORQ_VERSION_SERVER_MINOR && ver[3] == THORQ_VERSION_SERVER_PATCH;
	case THORQ_CMD_VERSION_CLIENT:
		return ver[1] == THORQ_VERSION_CLIENT_MAJOR && ver[2] == THORQ_VERSION_CLIENT_MINOR && ver[3] == THORQ_VERSION_CLIENT_PATCH;
	case THORQ_CMD_VERSION_LINK:
		return ver[1] == THORQ_VERSION_LINK_MAJOR && ver[2] == THORQ_VERSION_LINK_MINOR && ver[3] == THORQ_VERSION_LINK_PATCH;
	default:
		return false;
	}
}
inline std::string thorq_payload_version_to_string(const thorq_payload_t& payload)
{
	std::uint8_t ver[4]{0};

	thorq_payload_version_unpack(payload, ver[0], ver[1], ver[2], ver[3]);

	char buffer[12];
	memset(buffer, 0, 12);
	snprintf(buffer, 11, "%u.%u.%u", ver[1], ver[2], ver[3]);

	return std::string(buffer);
}

#endif // THORQ_PAYLOAD_VERSION_H
