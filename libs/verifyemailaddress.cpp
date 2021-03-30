#include "verifyemailaddress.h"

#include "bad_email_providers/bad_email_providers/bad_providers_generated.h"

#include "constants.h"

#include <cctype>
#include <string>
#include <atomic>
#include <istream>
#include <fstream>
#include <unordered_set>

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

inline bool validateRecepientString(char*& it)
{
    do {
        if (!IsRecepientChar(*it++)) {
            return false;
        }

        while (IsRecepientChar(*it)) { it++; }
    }
    while (*it++ == '.');

    return *(it - 1) == '@';
}

inline bool validateDomainString(char*& it)
{
    char c;

    do {
        if (!IsDomainChar(*it++)) {
            return false;
        }

        while (IsDomainChar(*it)) { it++; }

        c = *it++;
    }
    while (c == '-' || c == '.');

    it--;

    return c == 0;
}

bool IsThrowawayProvider(const std::string& provider)
{
    return is_bad_provider(provider);
}

bool ThorQ::Utils::IsEmailValid(std::string email)
{
    char* begin = email.data();
    char* end = begin + email.length();

    char* it = begin;

    // Check email size
    if (email.length() > THORQ_EMAIL_LEN_MAX || email.length() < THORQ_EMAIL_LEN_MIN) {
        return false;
    }

    // Verify that email recepient section is valid
    if (!validateRecepientString(it)) {
        return false;
    }

    // The domain section is case insensitive, so convert it to lowercase
    std::transform(it, end, it, &tolower);

    // Check against known throwaway email-provider domains
    if (IsThrowawayProvider(std::string(it, end))) {
        return false;
    }

    // Validate domain validity
    if (!validateDomainString(it)) {
        return false;
    }

    // Check that the entire email has been checked
    std::size_t validatedLength = it - begin;
    return validatedLength == email.length();
}

bool ThorQ::Utils::NormalizeEmail(std::string& email)
{
    char* begin = email.data();
    char* end = begin + email.length();

    char* it = begin;

    // Skip email receptient section
    if (!validateRecepientString(it)) {
        return false;
    }

    std::transform(it, end, it, &tolower);

    return true;
}
