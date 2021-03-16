#ifndef PASSWORDHASHING_H
#define PASSWORDHASHING_H

#define SODIUM_STATIC
#include <sodium.h>

#include <span>
#include <array>
#include <string>
#include <cstring>
#include <cstdint>

namespace ThorQ {
namespace Crypto {
namespace Hashing {
constexpr std::size_t SaltLength = crypto_pwhash_SALTBYTES;
constexpr std::size_t HashLength = crypto_box_SEEDBYTES;

static_assert (Hashing::SaltLength == 16, "PasswordHashing salt length changed");
static_assert (Hashing::HashLength == 32, "PasswordHashing hash length changed");

typedef std::array<std::uint8_t, Hashing::HashLength> Hash;
typedef std::span<std::uint8_t, Hashing::HashLength> HashRef;
typedef std::array<std::uint8_t, Hashing::SaltLength> Salt;
typedef std::span<std::uint8_t, Hashing::SaltLength> SaltRef;
struct Parameters {
    std::int64_t ops_limit;
    std::int64_t mem_limit;
    std::int32_t algorithm;

    enum class Performance {
        Interactive,
        Moderate,
        Sensitive
    };

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

    inline bool operator == (const Parameters& other) const {
        return (ops_limit == other.ops_limit) &&
               (mem_limit == other.mem_limit) &&
               (algorithm == other.algorithm);
    }
    inline bool operator != (const Parameters& other) const {
        return !(*this == other);
    }
};

inline void generateSalt(Hashing::Salt& salt)
{
    randombytes_buf(salt.data(), salt.size());
}

[[nodiscard]] inline bool Generate(const char* password, std::size_t passwordLen, const std::uint8_t* salt, const Hashing::Parameters& parameters, std::uint8_t* hashOut) {
    return crypto_pwhash(hashOut, Hashing::HashLength, password, passwordLen, salt, parameters.ops_limit, parameters.mem_limit, parameters.algorithm) == 0;
}
[[nodiscard]] inline bool Generate(const std::string& password, const Hashing::Salt& salt, const Hashing::Parameters& parameters, std::uint8_t* hashOut) {
    return Hashing::Generate(password.data(), password.size(), salt.data(), parameters, hashOut);
}
[[nodiscard]] inline bool Generate(const std::string& password, const Hashing::Salt& salt, const Hashing::Parameters& parameters, std::span<std::uint8_t, Hashing::HashLength> hashOut) {
    return Hashing::Generate(password, salt, parameters, hashOut.data());
}
}
}
}

#endif // PASSWORDHASHING_H
