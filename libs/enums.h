#ifndef ENUMS_H
#define ENUMS_H

#include <cstdint>

enum class THORQ_APP : std::uint8_t
{
    _INVALID,

	THORQ_APP_SERVER,
	THORQ_APP_CLIENT,
	THORQ_APP_LINK,

    _MAX
};

enum class THORQ_CHANNEL : std::uint8_t
{
    _INVALID,

    MAIN,      ///< Main channel (login/logout/friend/request)
    EVENTS,    ///< Events (status/relation)
    IMPULSE,   ///< Impulse data (collar/toys)
    AUTHORITY, ///< Moderations/Announcements/Admin

    _MAX
};

/// Flags to describe the payload of a message
enum class THORQ_PAYLOAD_ID : std::uint8_t
{
    _INVALID,

    // Main & Event channel
    HEARTBEAT,    ///< Heartbeat to keep connection alive and determine RTT
    VERSION,      ///< Request updated version info
    CRYPTO,       ///< Cryptographic handshake messages
    SYSTEMID,     ///< SystemID messages
    ACCOUNT,      ///< Create/Delete/Recover/Login/Logout
    RELATIONSHIP, ///< Friend/Block/AcceptFriend/DenyFriend/UnFriend/GetRelations
    SESSION,      ///< Sessions with other people
    ACK,          ///< Acknowledge

    // Impulse channel
    TOY,          ///< Toys
    COLLAR,       ///< Collars

    // Authority channel
    MODERATION,   ///< Bans/Reporting
    ANNOUNCEMENT, ///< Server notifications/Admin notifications

    _MAX
};


/// Acknowledgement of message sent from remote host
enum class THORQ_PAYLOAD_ACK : std::uint8_t
{
    _INVALID,

    OK,            ///< Command succeeded
    IN_PROGRESS,   ///< Command accepted, and is in progress
    NO_CHANGE,     ///< Command was ignored, because it didnt change anything

    DENIED,        ///< Command was denied
    INVALID,       ///< Command itself or its format is invalid
    LOGIN_NEEDED,  ///< Client has not logged in
    UNAUTHORIZED,  ///< Client has not authenticated (Crypto + Auth)

    ERROR_OCCURED, ///< Server experienced an error executing command

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
    UNKNOWN = 0,
    VERSION_INCOMPATIBLE,

    CRYPTO_FAILED,

    SHUTDOWN_CLOSED,
    SHUTDOWN_MAINTANENCE,

    KICKED,
    BANNED,
    TIMED_OUT,

    FUCK_YOU
};

////////////////////////////////////////////////////
/// USER STATES
////////////////////////////////////////////////////

/// Friend relationship status
enum class THORQ_RELATIONSHIP_STATUS : std::uint8_t
{
    BLOCKED, ///< Target is blocked
    NONE,    ///< No relationship status
    PENDING, ///< Outgoing friend-request
    FRIENDS, ///< Account is friended
};

/// Actions to be taken on session requested
enum class THORQ_RELATIONSHIP_AUTHORITY : std::uint8_t
{
    REJECT,   ///< Always reject this persons requests
    SILENT,   ///< Persons requests will not prompt me
    NOTIFY,   ///< Prompt me if this person requests control
    ACCEPT,   ///< Accept if this person requests
    EXCLUSIVE ///< This person can invoke exclusive access to me (will kick everyone else out)
};

enum THORQ_ACCOUNT_AUTHORITY : std::uint8_t
{
    THORQ_ACCOUNT_AUTHORITY_NONE,          ///< Just a normie
    THORQ_ACCOUNT_AUTHORITY_MODERATOR,     ///< Can moderate users (kick/ban)
    THORQ_ACCOUNT_AUTHORITY_ADMINISTRATOR, ///< Can manage server parameters
    THORQ_ACCOUNT_AUTHORITY_FOUNDER        ///< Exclusive to HeavenVR
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
