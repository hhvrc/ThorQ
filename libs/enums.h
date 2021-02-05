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
enum class CryptoLinkStatus
{
    None,
    Establishing,
    Active
};
/// @enum THORQ_STATE_HWID
/// State machine for client hwid authentication
enum THORQ_STATE_HWID
{
    THORQ_STATE_HWID_NONE,// = THORQ_STATE_CRYPTO_ACTIVE,          ///< Client has not been authenticated yet
    THORQ_STATE_HWID_REQUESTING,                                ///< Server has requested hardwareID from client / Client has sent SystemID to server and is awaiting a response
    THORQ_STATE_HWID_OK,                                        ///< Server authenticated client, client can now access the api
};

/// State machine for login
enum THORQ_STATE_LOGIN
{
    THORQ_STATE_LOGIN_LOGGEDOUT = THORQ_STATE_HWID_OK,          ///< Client is logged out
	THORQ_STATE_LOGIN_LOGGINGOUT,                               ///< Client has requested the server to log it out gracefully
	THORQ_STATE_LOGIN_LOGGINGIN,                                ///< Client has requested to log in with a username, and waiting for the server to accept
	THORQ_STATE_LOGIN_LOGGEDIN,                                 ///< Client is logged in with a username, and is discoverable by other online users
};

/// State machine for session
enum THORQ_STATE_SESSION
{
    THORQ_STATE_SESSION_NONE = THORQ_STATE_LOGIN_LOGGEDIN, ///< Host is not currently in a session
    THORQ_STATE_SESSION_LEAVING,                           ///< Host is leaving a session
    THORQ_STATE_SESSION_JOINING,                           ///< The session has been accepted and the host is waiting for the server to start it
    THORQ_STATE_SESSION_ACTIVE,                            ///< Both partners are currently in a session
};

#endif // ENUMS_H
