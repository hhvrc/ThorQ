#ifndef SCHEMAS_COMMON_H
#define SCHEMAS_COMMON_H

#if defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#elif defined(_MSC_VER)
#pragma warning( push, 1 )
#endif

#include <schemas/account_generated.h>
#include <schemas/announcement_generated.h>
#include <schemas/crypto_generated.h>
#include <schemas/device_generated.h>
#include <schemas/file_generated.h>
#include <schemas/friendrequest_generated.h>
#include <schemas/group_generated.h>
#include <schemas/message_generated.h>
#include <schemas/moderation_generated.h>
#include <schemas/p2p_generated.h>
#include <schemas/systemid_generated.h>
#include <schemas/user_generated.h>
#include <schemas/uuid_generated.h>
#include <schemas/version_generated.h>

#if defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning( pop )
#endif

#endif // SCHEMAS_COMMON_H
