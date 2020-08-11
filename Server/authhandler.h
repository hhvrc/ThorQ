#ifndef AUTHHANDLER_H
#define AUTHHANDLER_H

#include <set>
#include <map>
#include <vector>
#include <cstdint>

#include <constants.h>

namespace ThorQ {
namespace AuthHandler {
bool tryAddRegkey(const std::array<std::uint8_t, THORQ_AUTH_REGKEY_LEN>& key);
void removeRegkey(const std::array<std::uint8_t, THORQ_AUTH_REGKEY_LEN>& key);

enum ResponseCode
{
	REGISTERED,
	RE_REGISTERED,
	NOT_REGISTERED,

	TIMEOUT,
	INVALID_REGKEY,
	INVALID_SYSTEMID,
};
ResponseCode checkSystemID(const std::vector<std::uint8_t>& hwid);
ResponseCode tryRegisterSystemID(const std::vector<std::uint8_t>& hwid, const std::array<std::uint8_t, THORQ_AUTH_REGKEY_LEN>& key);
}
}

#endif // AUTHHANDLER_H
