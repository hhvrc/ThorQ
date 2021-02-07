#ifndef ENUMS_H
#define ENUMS_H

#include <cstdint>

enum class THORQ_APP : std::uint8_t
{
    _INVALID,

    SERVER,
    CLIENT,
    LINK,

    _MAX
};

/// Id of a device or service that client has
enum THORQ_USER_ACTIVITY_FLAG : std::uint8_t
{
    THORQ_USER_ACTIVITY_FLAG_COLLAR_PRESENT = 1 << 0, ///< User currently has a collar connected to their pc, this will show up as a [🗲] icon next to their name
    THORQ_USER_ACTIVITY_FLAG_OPENVR_RUNNING = 1 << 1, ///< User is currently in Virtual Reality, this will show up as a [VR] icon next to their name
    THORQ_USER_ACTIVITY_FLAG_RESERVED_3     = 1 << 2,
    THORQ_USER_ACTIVITY_FLAG_RESERVED_4     = 1 << 3,
    THORQ_USER_ACTIVITY_FLAG_RESERVED_5     = 1 << 4,
    THORQ_USER_ACTIVITY_FLAG_RESERVED_6     = 1 << 5,
    THORQ_USER_ACTIVITY_FLAG_RESERVED_7     = 1 << 6,
    THORQ_USER_ACTIVITY_FLAG_RESERVED_8     = 1 << 7,
};

///
enum class THORQ_DISCONNECT_REASON : std::uint32_t
{
    UNKNOWN,
    VERSION_INCOMPATIBLE,

    CRYPTO_FAILED,

    SHUTDOWN_CLOSED,
    SHUTDOWN_MAINTANENCE,

    KICKED,
    BANNED,
    TIMED_OUT,

    _MAX
};

enum class ConnectionStatus
{
    Error,
    Disconnected,
    Connecting,
    Connected,
    Disconnecting
};
enum class ProcessStatus
{
    Stopped,
    Starting,
    Running,
    Stopping
};

#endif // ENUMS_H
