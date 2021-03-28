#include "verifyemailaddress.h"

#include "constants.h"

constexpr bool IsRecepientChar(char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') ||
           c == '!'  ||
           c == '#'  ||
           c == '$'  ||
           c == '%'  ||
           c == '&'  ||
           c == '\'' ||
           c == '*'  ||
           c == '+'  ||
           c == '/'  ||
           c == '='  ||
           c == '?'  ||
           c == '^'  ||
           c == '_'  ||
           c == '`'  ||
           c == '{'  ||
           c == '|'  ||
           c == '}'  ||
           c == '~'  ||
           c == '-';
}
constexpr bool IsDomainChar(char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9');
}

inline bool validateRecepientString(char*& ptr)
{
    do {
        if (!IsRecepientChar(*ptr++)) {
            return false;
        }

        while (IsRecepientChar(*ptr)) { ptr++; }
    }
    while (*ptr++ == '.');

    return *(ptr - 1) == '@';
}

inline bool validateDomainString(char*& ptr)
{
    char c;

    do {
        if (!IsDomainChar(*ptr++)) {
            return false;
        }

        while (IsDomainChar(*ptr)) { ptr++; }

        c = *ptr++;
    }
    while (c == '-' || c == '.');

    ptr--;

    return c == 0;
}

bool ThorQ::Utilities::IsEmailValid(const char* email, std::size_t emailLen)
{
    char* it = (char*)email;

    if (!validateRecepientString(it)) {
        return false;
    }

    if (!validateDomainString(it)) {
        return false;
    }

    std::size_t validatedLength = it - email;

    return validatedLength == emailLen &&
           validatedLength <= THORQ_EMAIL_LEN_MAX &&
           validatedLength >= THORQ_EMAIL_LEN_MIN;
}
