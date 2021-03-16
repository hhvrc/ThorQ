#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "version.h"

#include <cstddef>
#include <cstdint>

#define THORQ_VERSION_SERVER_MAJOR 0
#define THORQ_VERSION_SERVER_MINOR 0
#define THORQ_VERSION_SERVER_PATCH 132

#define THORQ_VERSION_CLIENT_MAJOR 0
#define THORQ_VERSION_CLIENT_MINOR 0
#define THORQ_VERSION_CLIENT_PATCH 132

#define THORQ_VERSION_LINK_MAJOR 0
#define THORQ_VERSION_LINK_MINOR 0
#define THORQ_VERSION_LINK_PATCH 132

namespace ThorQ {
const ThorQ::Version ServerVersion(THORQ_VERSION_SERVER_MAJOR, THORQ_VERSION_SERVER_MINOR, THORQ_VERSION_SERVER_PATCH);
const ThorQ::Version ClientVersion(THORQ_VERSION_CLIENT_MAJOR, THORQ_VERSION_CLIENT_MINOR, THORQ_VERSION_CLIENT_PATCH);
const ThorQ::Version LinkVersion(THORQ_VERSION_LINK_MAJOR, THORQ_VERSION_LINK_MINOR, THORQ_VERSION_LINK_PATCH);
}

/*
 *
 */
constexpr std::uint32_t THORQ_PAYLOAD_LEN_MAX = 1024 * 512; ///< Maximum payload length
constexpr std::uint32_t THORQ_PAYLOAD_LEN_TYP = 1024;       ///< Typical payload length
constexpr std::uint32_t THORQ_PAYLOAD_LEN_MIN = 1;          ///< Minimum payload length

/* So according to the POSIX standard, a hostname is guaranteed not to exceed 255 bytes
 * Also a hostname can minimum be 1 character long
 *
 * The systemID we generate is 10 bytes long, so this sums up to:
 * Minimum: 10 + 1   (11)
 * Maximum: 10 + 255 (265)
 */
constexpr std::size_t THORQ_AUTH_SYSTEMID_LEN_MIN = 11; ///< Minimum SystemID length (String is 26)
constexpr std::size_t THORQ_AUTH_SYSTEMID_LEN_MAX = 265; ///< Maximum SystemID length (String is 280)

/* Users cannot have usernames less than 2 characters, thats retarded
 * Limit usernames at 32 characters, because having more is... retarded
 */
constexpr int THORQ_USERNAME_LEN_MIN = 2; ///< Minimum Username length
constexpr int THORQ_USERNAME_LEN_MAX = 32; ///< Maximum Username length

/* Length of auth token
 */
constexpr int THORQ_AUTHTOKEN_LEN = 256;

/* Having a password less than 6 characters long is stupid and i wont allow supidity, more than 128 chars is also stupid
 */
constexpr int THORQ_PASSWORD_LEN_MIN = 6; ///< Minimum Password length
constexpr int THORQ_PASSWORD_LEN_MAX = 128; ///< Maximum Password length

/* Email address limits
 */
constexpr int THORQ_EMAIL_LEN_MIN = 6; ///< Minimum id length (x@y.zz)
constexpr int THORQ_EMAIL_LEN_MAX = 254; ///< Maximum id length (https://web.archive.org/web/20130710170052/http://www.eph.co.uk/resources/email-address-length-faq/)

/* Discord id limits
 */
constexpr int THORQ_DISCORDID_LEN_MIN = 5; ///< Minimum id length (1 char + "#xxxx")
constexpr int THORQ_DISCORDID_LEN_MAX = 37; ///< Maximum id length (32 chars + "#xxxx")

/* TODO: move this to a config file
 */
#define THORQ_ORGANIZATION_NAME "HeavenVR"
#define THORQ_ORGANIZATION_DOMAIN "exampledomain.com"
#define THORQ_APPLICATION_NAME "ThorQ"
#define THORQ_APPLICATION_DESCRIPTION "Software to control 3rd party collars across the internet"
#define THORQ_SERVER_HOSTNAME "thorq.hentaiheaven.tech"
#define THORQ_SERVER_PORT 12345
#define THORQ_SERVER_MAX_CONNECTIONS 1024

#endif // CONSTANTS_H
