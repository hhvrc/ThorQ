#ifndef HASHER_H
#define HASHER_H

#include <string>
#include <future>

namespace ThorQ {
namespace Security {
namespace Hashing {
std::future<std::string> Hash(const std::string& password);
std::future<bool> Verify(const std::string &password, const std::string& hash);
}
}
}

#endif // HASHER_H
