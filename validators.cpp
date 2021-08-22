#include "validators.h"

#include "constants.h"

#include <QRegExp>

ThorQ::Validators::RetType ThorQ::Validators::UsernameValidator(ThorQ::Validators::ArgType username)
{
    if (username.length() < ThorQ::USERNAME_LEN_MIN) {
        return ThorQ::Validators::RetType(false, "Too long username");
    }

    if (username.length() < ThorQ::USERNAME_LEN_MIN) {
        return ThorQ::Validators::RetType(false, "Too short username");
    }

    return ThorQ::Validators::DefaultValue;
}

ThorQ::Validators::RetType ThorQ::Validators::PasswordValidator(ThorQ::Validators::ArgType password)
{
    if (password.length() < ThorQ::PASSWORD_LEN_MIN) {
        return ThorQ::Validators::RetType(false, "Too long password");
    }

    if (password.length() < ThorQ::PASSWORD_LEN_MIN) {
        return ThorQ::Validators::RetType(false, "Too short password");
    }

    return ThorQ::Validators::DefaultValue;
}


constexpr bool IsAlhaNumericChar(QChar c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9');
}
constexpr bool IsValidLocalChar(QChar  c)
{
    return IsAlhaNumericChar(c) ||
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
constexpr bool IsValidLocalPart(const QChar*& it)
{
    const QChar* startIt = it;
    do {
        if (!IsValidLocalChar(*it++)) {
            return false;
        }

        while (IsValidLocalChar(*it)) { it++; }
    }
    while (*it++ == '.');

    int localLen = (it - 1) - startIt;

    return *(it - 1) == '@' && localLen <= 64;
}
constexpr bool IsValidDomainPart(const QChar*& it)
{
    QChar c = 0;
    do {
        const QChar* startIt = it;
        if (!IsAlhaNumericChar(*it++)) {
            return false;
        }

        while (IsAlhaNumericChar(*it)) { it++; }

        int dnsLabelLen = it - startIt;
        if (dnsLabelLen > 63) {
            return false;
        }

        c = *it++;
    }
    while (c == '-' || c == '.');

    it--;

    return c == 0;
}

ThorQ::Validators::RetType ThorQ::Validators::EmailValidator(ThorQ::Validators::ArgType email)
{/*
    if (email.length() > ThorQ::EMAIL_LEN_MAX) {
        return ThorQ::Validators::RetType(false, "Too long email");
    }

    if (email.length() < ThorQ::EMAIL_LEN_MIN) {
        return ThorQ::Validators::RetType(false, "Invalid email");
    }

    QRegExp mailRegex("\\b[a-z0-9._%+-]+@[a-z0-9.-]+\\.[a-z]{2,4}\\b");
    mailRegex.setCaseSensitivity(Qt::CaseInsensitive);
    mailRegex.setPatternSyntax(QRegExp::RegExp);

    if (!mailRegex.exactMatch(email.toString())) {
        return ThorQ::Validators::RetType(false, "Invalid email");
    }
*/
    const QChar* begin = email.data();
    const QChar* it = begin;

    // Check email size
    if (email.length() > ThorQ::EMAIL_LEN_MAX) {
        return ThorQ::Validators::RetType(false, "Too long email");
    }
    if (email.length() < ThorQ::EMAIL_LEN_MIN) {
        return ThorQ::Validators::RetType(false, "Invalid email");
    }

    // Verify that email recepient section is valid
    if (!IsValidLocalPart(it)) {
        return ThorQ::Validators::RetType(false, "Email user name invalid");
    }

    // Validate domain validity
    if (!IsValidDomainPart(it)) {
        return ThorQ::Validators::RetType(false, "Email domain name invalid");
    }

    // Check that the entire email has been checked
    int validatedLength = it - begin;

    if (validatedLength != email.length()) {
        return ThorQ::Validators::RetType(false, "Invalid email");
    }

    return ThorQ::Validators::DefaultValue;
}
