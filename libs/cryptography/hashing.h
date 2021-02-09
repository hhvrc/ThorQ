#ifndef PASSWORDHASHING_H
#define PASSWORDHASHING_H

#include <sodium.h>

#include <span>
#include <string>
#include <cstdint>

namespace ThorQ {
namespace Crypto {
namespace Hashing {
constexpr std::size_t SaltLength = crypto_pwhash_SALTBYTES;
constexpr std::size_t HashLength = crypto_box_SEEDBYTES;

static_assert (Hashing::SaltLength == 16, "PasswordHashing salt length changed");
static_assert (Hashing::HashLength == 32, "PasswordHashing hash length changed");

struct HashingParameters {
    std::array<std::uint8_t, Hashing::SaltLength> salt;
    std::size_t ops_limit;
    std::size_t mem_limit;
    int algorithm;

    enum class Performance {
        Interactive,
        Moderate,
        Sensitive
    };

    inline void randomizeSeed() {
        randombytes_buf(salt.data(), salt.size());
    }
    constexpr void setPerformance(Performance perf) {
        switch (perf) {
        case Performance::Interactive:
            ops_limit = crypto_pwhash_OPSLIMIT_INTERACTIVE;
            mem_limit = crypto_pwhash_MEMLIMIT_INTERACTIVE;
            algorithm = crypto_pwhash_ALG_DEFAULT;
            break;
        case Performance::Moderate:
            ops_limit = crypto_pwhash_OPSLIMIT_MODERATE;
            mem_limit = crypto_pwhash_MEMLIMIT_MODERATE;
            algorithm = crypto_pwhash_ALG_DEFAULT;
            break;
        case Performance::Sensitive:
            ops_limit = crypto_pwhash_OPSLIMIT_SENSITIVE;
            mem_limit = crypto_pwhash_MEMLIMIT_SENSITIVE;
            algorithm = crypto_pwhash_ALG_DEFAULT;
            break;
        }
    }
};

[[nodiscard]] inline bool Generate(const char* password, std::size_t passwordLen, const Hashing::HashingParameters& parameters, std::uint8_t* hashOut) {
    return crypto_pwhash(hashOut, Hashing::HashLength, password, passwordLen, parameters.salt.data(), parameters.ops_limit, parameters.mem_limit, parameters.algorithm);
}
[[nodiscard]] inline bool Generate(const std::string& password, const Hashing::HashingParameters& parameters, std::uint8_t* hashOut) {
    return Hashing::Generate(password.data(), password.size(), parameters, hashOut);
}
[[nodiscard]] inline bool Generate(const std::string& password, const Hashing::HashingParameters& parameters, std::span<std::uint8_t, Hashing::HashLength> hashOut) {
    return Hashing::Generate(password, parameters, hashOut.data());
}
}
}
}

#endif // PASSWORDHASHING_H
