#ifndef MSG_MESSAGE_H
#define MSG_MESSAGE_H

#include <cstdint>
#include <memory>
#include "constants.h"
#include "crypto.h"

// Macro to define packed structures
#ifdef __GNUC__
  #define THORQPACKED( __Declaration__ ) __Declaration__ __attribute__((packed))
#else
  #define THORQPACKED( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#endif

/**
 * @brief The thorq_message_t struct
 */
typedef struct __thorq_msg
{
    __thorq_msg();
    __thorq_msg(const __thorq_msg& other);
    __thorq_msg& operator = (const __thorq_msg& other);

    std::uint8_t flags = 0;
    std::vector<std::uint8_t> payload; ///< First byte is the message id
    std::uint8_t payload_iv[THORQ_CRYPTO_CIPHER_IV_LEN]{0};

    bool operator == (const __thorq_msg& other) const;
    bool operator != (const __thorq_msg& other) const;
} thorq_msg_t;

constexpr std::size_t THORQ_MSG_SIZE_MIN = 2;
constexpr std::size_t THORQ_MSG_SIZE_MAX = THORQ_MSG_MAX_PAYLOAD_LEN + THORQ_CRYPTO_CIPHER_IV_LEN + 1;


static bool thorq_msg_is_valid(const std::uint8_t* data, std::size_t size);
static bool thorq_msg_is_encrypted(const thorq_msg_t* msg);
static bool thorq_msg_encrypt(thorq_msg_t* msg, ThorQ::Crypto* crypto);
static bool thorq_msg_decrypt(thorq_msg_t* msg, ThorQ::Crypto* crypto);

static bool thorq_msg_encode(const thorq_msg_t* msg, std::vector<std::uint8_t>* data);
static bool thorq_msg_decode(const std::uint8_t* data, std::size_t size, thorq_msg_t* msg);

static std::uint8_t thorq_msg_get_msg_id(const thorq_msg_t* msg);

#endif // MSG_MESSAGE_H
