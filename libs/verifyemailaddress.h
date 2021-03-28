#ifndef VERIFYEMAILADDRESS_H
#define VERIFYEMAILADDRESS_H

#include <string>

namespace ThorQ {
namespace Utilities {
bool IsEmailValid(const char* email, std::size_t emailLen);
inline bool IsEmailValid(const std::string& email) { return ThorQ::Utilities::IsEmailValid(email.data(), email.length()); }
inline bool IsEmailValid(std::string_view email) { return ThorQ::Utilities::IsEmailValid(email.data(), email.length()); };
}
}

#endif // VERIFYEMAILADDRESS_H
