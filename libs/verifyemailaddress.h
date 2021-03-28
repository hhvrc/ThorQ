#ifndef VERIFYEMAILADDRESS_H
#define VERIFYEMAILADDRESS_H

#include <string>

namespace ThorQ::Utils {
bool IsEmailValid(std::string email);
bool NormalizeEmail(std::string& email);
}

#endif // VERIFYEMAILADDRESS_H
