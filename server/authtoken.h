#ifndef AUTHTOKEN_H
#define AUTHTOKEN_H

#include <array>
#include <vector>
#include <chrono>
#include <cstdint>

#include <crypto.h>
#include <systemid.h>

struct AuthToken
{
    std::uint8_t token[32]; // 256-bit Hex-Encoded authtoken
    std::vector<std::uint8_t> systemID;
    std::chrono::time_point createdAt; // YYYY/MM/DD/HH/MM/SS
};

inline AuthToken CreateAuthToken(const std::vector<std::uint8_t>& systemID)
{
    AuthToken authToken;

    ThorQ::Crypto::RandomizeBytes(authToken.token, 32);

    authToken.systemID = systemID;
    authToken.createdAt = std::chrono::now();

    return authToken;
}

#endif // AUTHTOKEN_H
