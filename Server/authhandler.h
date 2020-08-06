#ifndef AUTHHANDLER_H
#define AUTHHANDLER_H

#include <set>
#include <map>
#include <vector>
#include <cstdint>

namespace ThorQ {
namespace AuthHandler {
bool CheckSystemID(const std::string& hwid);
bool TryRegisterHwid(const std::string& hwid, const std::vector<std::uint8_t>& key);
}
}

#endif // AUTHHANDLER_H
