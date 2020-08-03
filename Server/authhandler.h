#ifndef AUTHHANDLER_H
#define AUTHHANDLER_H

#include <set>

class AuthHandler
{
    std::set<std::uint8_t[34]> auth;
public:
    AuthHandler();
};

#endif // AUTHHANDLER_H
