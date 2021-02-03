#ifndef AUTHTOKEN_H
#define AUTHTOKEN_H

#include <systemid.h>

#include <sodium.h>

#include <array>
#include <vector>
#include <chrono>
#include <cstdint>

class AuthToken
{
public:
    AuthToken CreateToken(const std::vector<std::uint8_t>& systemID);
private:
    std::uint8_t token[32]; // 256-bit Hex-Encoded authtoken
    std::vector<std::uint8_t> systemID;
    std::chrono::system_clock::time_point createdAt; // YYYY/MM/DD/HH/MM/SS
};

inline AuthToken AuthToken::CreateToken(const std::vector<uint8_t> &systemID)
{
    AuthToken authToken;

    randombytes_buf(authToken.token, 32);

    authToken.systemID = systemID;
    authToken.createdAt = std::chrono::system_clock::now();

    return authToken;
}

#endif // AUTHTOKEN_H
