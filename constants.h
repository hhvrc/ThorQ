#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstddef>
#include <cstdint>

#define THORQ_APPLICATION_NAME "ThorQ"
#define THORQ_ORGANIZATION_NAME "HeavenVR"
#define THORQ_ORGANIZATION_DOMAIN "heavenvr.tech"
#define THORQ_APPLICATION_DESCRIPTION "OpenSource program to control a shock-collar remotely from anywhere in the world"
#define THORQ_SERVER_HOSTNAME "https://api.heavenvr.tech/thorq"

namespace ThorQ {
constexpr std::uint16_t VERSION_MAJOR = 0;
constexpr std::uint16_t VERSION_MINOR = 0;
constexpr std::uint16_t VERSION_PATCH = 132;

/* Users cannot have usernames less than 2 characters, thats retarded
 * Limit usernames at 32 characters, because having more is... retarded
 */
constexpr int USERNAME_LEN_MIN = 2; ///< Minimum Username length
constexpr int USERNAME_LEN_MAX = 32; ///< Maximum Username length

/* Length of auth token
 */
constexpr int AUTHTOKEN_LEN = 256;

/* Having a password less than 6 characters long is stupid and i wont allow supidity, more than 128 chars is also stupid
 */
constexpr int PASSWORD_LEN_MIN = 6; ///< Minimum Password length
constexpr int PASSWORD_LEN_MAX = 128; ///< Maximum Password length

/* Email address limits
 */
constexpr int EMAIL_LEN_MIN = 6; ///< Minimum id length (x@y.zz)
constexpr int EMAIL_LEN_MAX = 254; ///< Maximum id length (https://web.archive.org/web/20130710170052/http://www.eph.co.uk/resources/email-address-length-faq/)

/* Discord id limits
 */
constexpr int DISCORDID_LEN_MIN = 5; ///< Minimum id length (1 char + "#xxxx")
constexpr int DISCORDID_LEN_MAX = 37; ///< Maximum id length (32 chars + "#xxxx")

constexpr const char* APPLICATION_NAME = THORQ_APPLICATION_NAME;
constexpr const char* ORGANIZATION_NAME = THORQ_ORGANIZATION_NAME;
constexpr const char* ORGANIZATION_DOMAIN = THORQ_ORGANIZATION_DOMAIN;
constexpr const char* APPLICATION_DESCRIPTION = THORQ_APPLICATION_DESCRIPTION;
constexpr const char* SERVER_HOSTNAME = THORQ_SERVER_HOSTNAME;
}

#endif // CONSTANTS_H
