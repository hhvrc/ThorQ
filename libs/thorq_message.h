#ifndef MSG_MESSAGE_H
#define MSG_MESSAGE_H

#include <cstdint>
#include <memory>
#include "constants.h"

// Macro to define packed structures
#ifdef __GNUC__
  #define THORQPACKED( __Declaration__ ) __Declaration__ __attribute__((packed))
#else
  #define THORQPACKED( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#endif

/**
 * @brief The thorq_message_t struct
 */
THORQPACKED(
typedef struct __thorq_message
{
	std::uint8_t flags = 0;
	std::uint8_t msg_id = 0; ///< THORQ_MSG_ID
	std::uint8_t payload[THORQ_MSG_MAX_PAYLOAD_LEN]{0};
	std::uint8_t payload_iv[THORQ_CRYPTO_CIPHER_IV_LEN]{0};

	bool operator == (const __thorq_message& other) const;
	bool operator != (const __thorq_message& other) const;
}) thorq_message_t;

#endif // MSG_MESSAGE_H
