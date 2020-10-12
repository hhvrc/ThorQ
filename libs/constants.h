#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>
#include <cstddef>

#include "version.h"

constexpr std::uint8_t THORQ_VERSION_SERVER_MAJOR = 0;
constexpr std::uint8_t THORQ_VERSION_SERVER_MINOR = 0;
constexpr std::uint8_t THORQ_VERSION_SERVER_PATCH = 132;
constexpr ThorQ::Version THORQ_VERSION_SERVER { THORQ_VERSION_SERVER_MAJOR, THORQ_VERSION_SERVER_MINOR, THORQ_VERSION_SERVER_PATCH };

constexpr std::uint8_t THORQ_VERSION_CLIENT_MAJOR = 0;
constexpr std::uint8_t THORQ_VERSION_CLIENT_MINOR = 0;
constexpr std::uint8_t THORQ_VERSION_CLIENT_PATCH = 132;
constexpr ThorQ::Version THORQ_VERSION_CLIENT { THORQ_VERSION_CLIENT_MAJOR, THORQ_VERSION_CLIENT_MINOR, THORQ_VERSION_CLIENT_PATCH };

constexpr std::uint8_t THORQ_VERSION_LINK_MAJOR = 0;
constexpr std::uint8_t THORQ_VERSION_LINK_MINOR = 0;
constexpr std::uint8_t THORQ_VERSION_LINK_PATCH = 132;
constexpr ThorQ::Version THORQ_VERSION_LINK { THORQ_VERSION_LINK_MAJOR, THORQ_VERSION_LINK_MINOR, THORQ_VERSION_LINK_PATCH };

/*
 *
 */
constexpr std::size_t THORQ_PAYLOAD_LEN_MAX = 512; ///< Maximum payload length
constexpr std::size_t THORQ_PAYLOAD_LEN_MIN = 1;   ///< Minimum payload length

/* So according to the POSIX standard, a hostname is guaranteed not to exceed 255 bytes
 * Also a hostname can minimum be 1 character long
 *
 * The systemID we generate is 10 bytes long, so this sums up to:
 * Minimum: 10 + 1   (11)
 * Maximum: 10 + 255 (265)
 */
constexpr std::size_t THORQ_AUTH_SYSTEMID_LEN_MIN = 11; ///< Minimum SystemID length
constexpr std::size_t THORQ_AUTH_SYSTEMID_LEN_MAX = 265; ///< Maximum SystemID length

/* Users cannot have usernames less than 2 characters, thats retarded
 * Limit usernames at 32 characters, because having more is... retarded
 */
constexpr int THORQ_USERNAME_LEN_MIN = 2; ///< Minimum Username length
constexpr int THORQ_USERNAME_LEN_MAX = 32; ///< Maximum Username length

/* Length of auth token
 */
constexpr int THORQ_AUTHTOKEN_LEN = 256;

/* Having a password less than 6 characters long is stupid and i wont allow supidity
 * The hashing algorithm used effectively truncates the password at 72 characters, so set the limit there
 * see: https://botan.randombit.net/handbook/api_ref/passhash.html
 */
constexpr int THORQ_PASSWORD_LEN_MIN = 6; ///< Minimum Password length
constexpr int THORQ_PASSWORD_LEN_MAX = 72; ///< Maximum Password length

/* Discord id limits
 */
constexpr int THORQ_DISCORDID_LEN_MIN = 5; ///< Minimum id length (1 char + "#xxxx")
constexpr int THORQ_DISCORDID_LEN_MAX = 36; ///< Maximum id length (32 chars + "#xxxx")

/* TODO: move this to a config file
 */
constexpr const char*   THORQ_APPLICATION_NAME = "ThorQ";
constexpr const char*   THORQ_SERVER_HOSTNAME = "www.dededededede.de";

#endif // CONSTANTS_H
