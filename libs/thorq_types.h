#ifndef THORQ_TYPES_H
#define THORQ_TYPES_H

#include <cstdint>
#include <memory>

#include "constants.h"

// Macro to define packed structures
#ifdef __GNUC__
  #define THORQPACKED( __Declaration__ ) __Declaration__ __attribute__((packed))
#else
  #define THORQPACKED( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#endif

#define THORQ_MAX_PAYLOAD_LEN 255 ///< Maximum payload length

/**
 * @brief The thorq_version_t struct
 */
THORQPACKED(
typedef struct __thorq_version
{
    std::uint8_t major = 0;
    std::uint8_t minor = 0;
    std::uint8_t patch = 0;

    bool operator == (const __thorq_version& other) const
    {
        return memcmp(this, &other, sizeof(__thorq_version)) == 0;
    }
    bool operator != (const __thorq_version& other) const
	{
        return memcmp(this, &other, sizeof(__thorq_version)) != 0;
	}
}) thorq_version_t;

/**
 * @brief The thorq_payload_t struct
 */
THORQPACKED(
typedef struct __thorq_payload
{
    std::uint8_t meta = 0;
	std::uint8_t size = 0;
	std::uint8_t data[THORQ_MAX_PAYLOAD_LEN]{0};

    bool operator == (const __thorq_payload& other) const
	{
        return memcmp(this, &other, sizeof(__thorq_payload)) == 0;
	}
    bool operator != (const __thorq_payload& other) const
	{
        return memcmp(this, &other, sizeof(__thorq_payload)) != 0;
	}
}) thorq_payload_t;

/**
 * @brief The thorq_message_t struct
 */
THORQPACKED(
typedef struct __thorq_message
{
    std::uint8_t flags = 0;
    std::uint8_t msgid = 0;
    std::uint8_t payload[THORQ_MAX_PAYLOAD_LEN]{0};
    std::uint8_t payload_iv[THORQ_CRYPTO_IV_LENGTH]{0};

    bool operator == (const __thorq_message& other) const
	{
        return memcmp(this, &other, sizeof(__thorq_message)) == 0;
	}
    bool operator != (const __thorq_message& other) const
	{
        return memcmp(this, &other, sizeof(__thorq_message)) != 0;
	}
}) thorq_message_t;

/**
 * @brief The thorq_collar_t struct
 */
THORQPACKED(
typedef struct __thorq_collar
{
    std::uint8_t flags; ///< thorq_collar_flag_t
    std::uint8_t shock_value;
    std::uint8_t vibration_value;
    std::uint8_t beep_value;
    std::uint8_t auto_value;

    bool operator == (const __thorq_collar& other) const
    {
        return memcmp(this, &other, sizeof(__thorq_collar)) == 0;
    }
    bool operator != (const __thorq_collar& other) const
    {
        return memcmp(this, &other, sizeof(__thorq_collar)) != 0;
    }
}) thorq_collar_t;

constexpr thorq_version_t CurrentAppVersion
{
    THORQ_VERSION_APP_MAJOR,
    THORQ_VERSION_APP_MINOR,
    THORQ_VERSION_APP_PATCH
};
constexpr thorq_version_t CurrentLinkVersion
{
    THORQ_VERSION_LNK_MAJOR,
    THORQ_VERSION_LNK_MINOR,
    THORQ_VERSION_LNK_PATCH
};

#endif // THORQ_TYPES_H
