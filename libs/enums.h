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

enum class THORQ_CHANNEL : std::uint8_t
{
    _INVALID,

    API,       ///< For api queries, this channel should always use reliable packets (upgrade this to use TCP in the future)
    EVENTS,    ///< Api events?
    RTC,       ///< Real time communication (unsequenced, unreliable packet stream) this will be sent from peer to peer, and through server if the peers cant connect
    AUTHORITY, ///< Moderations/Announcements/Admin (remove me maybe?)

    _MAX
};

enum class ProcessStatus
{
    Error,
    Stopped,
    Starting,
    Running,
    Stopping
};
enum class ConnectionStatus
{
    Error,
    Disconnected,
    Connecting,
    Connected,
    Disconnecting
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

////////////////////////////////////////////////////
/// STATE MACHINES
////////////////////////////////////////////////////

/// State machine for connection
enum THORQ_STATE_CONNECTION
{
	THORQ_STATE_CONNECTION_DISCONNECTED,  ///< Host is disconnected
	THORQ_STATE_CONNECTION_DISCONNECTING, ///< Host has requested that the server disconnects it gracefully
	THORQ_STATE_CONNECTION_CONNECTING,    ///< Host is connecting
	THORQ_STATE_CONNECTION_CONNECTED,     ///< Host is connected
};

/// @enum THORQ_STATE_CRYPTO
/// State machine for crypto
enum THORQ_STATE_CRYPTO
{
	THORQ_STATE_CRYPTO_NONE = THORQ_STATE_CONNECTION_CONNECTED, ///< The cryptographic link with the other host has not been established yet
	THORQ_STATE_CRYPTO_REQUESTED,                               ///< A request has been sent to the other host, requesting to begin a handshake
	THORQ_STATE_CRYPTO_ESTABLISHING,                            ///< Hosts are now attempting to establish a shared secret
	THORQ_STATE_CRYPTO_VERIFYING,                               ///< Hosts are not trying to verify that they successfully agreed on a shared secret
	THORQ_STATE_CRYPTO_ACTIVE,                                  ///< Hosts have a shared secret and can send encrypted data between themselves
};

/// @enum THORQ_STATE_HWID
/// State machine for client hwid authentication
enum THORQ_STATE_HWID
{
    THORQ_STATE_HWID_NONE = THORQ_STATE_CRYPTO_ACTIVE,          ///< Client has not been authenticated yet
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
