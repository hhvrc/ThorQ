#ifndef ENUMS_H
#define ENUMS_H

#include <cstdint>

enum THORQ_APP : std::uint8_t
{
	THORQ_APP_SERVER,
	THORQ_APP_CLIENT,
	THORQ_APP_LINK,
};

enum THORQ_CHANNEL : std::uint8_t
{
    THORQ_CHANNEL_MAIN,      ///< Main channel (login/logout/friend/request)
    THORQ_CHANNEL_EVENTS,    ///< Events (status/relation)
    THORQ_CHANNEL_IMPULSE,   ///< Impulse data (collar/toys)
    THORQ_CHANNEL_AUTHORITY, ///< Moderations/Announcements/Admin
    THORQ_CHANNEL_COUNT
};

/// Flags to describe the payload of a message
enum THORQ_PAYLOAD_ID : std::uint8_t
{

    // Main & Event channel
    THORQ_PAYLOAD_ID_HEARTBEAT,    ///< Heartbeat to keep connection alive and determine RTT
    THORQ_PAYLOAD_ID_VERSION,      ///< Request updated version info
    THORQ_PAYLOAD_ID_CRYPTO,       ///< Cryptographic handshake messages
    THORQ_PAYLOAD_ID_SYSTEMID,     ///< SystemID messages
    THORQ_PAYLOAD_ID_ACCOUNT,      ///< Create/Delete/Recover/Login/Logout
    THORQ_PAYLOAD_ID_RELATIONSHIP, ///< Friend/Block/AcceptFriend/DenyFriend/UnFriend/GetRelations
    THORQ_PAYLOAD_ID_SESSION,      ///< Sessions with other people
    THORQ_PAYLOAD_ID_ACK,          ///< Acknowledge

    // Impulse channel
    THORQ_PAYLOAD_ID_TOY = 0, ///< Toys
    THORQ_PAYLOAD_ID_COLLAR,  ///< Collars

    // Authority channel
    THORQ_PAYLOAD_ID_MODERATION = 0, ///< Bans/Reporting
    THORQ_PAYLOAD_ID_ANNOUNCEMENT,   ///< Server notifications/Admin notifications

    THORQ_PAYLOAD_ID__MAX = THORQ_PAYLOAD_ID_ACK,
    THORQ_PAYLOAD_ID__INVALID,
};


/// Acknowledgement of message sent from remote host
enum THORQ_PAYLOAD_ACK : std::uint8_t
{
	THORQ_PAYLOAD_ACK_OK,           ///< Command succeeded
	THORQ_PAYLOAD_ACK_IN_PROGRESS,  ///< Command accepted, and is in progress
	THORQ_PAYLOAD_ACK_NO_CHANGE,    ///< Command was ignored, because it didnt change anything

	THORQ_PAYLOAD_ACK_DENIED,       ///< Command was denied
    THORQ_PAYLOAD_ACK_INVALID,      ///< Command itself or its format is invalid
	THORQ_PAYLOAD_ACK_LOGIN_NEEDED, ///< Client has not logged in
	THORQ_PAYLOAD_ACK_UNAUTHORIZED, ///< Client has not authenticated (Crypto + Auth)

    THORQ_PAYLOAD_ACK_ERROR         ///< Server experienced an error executing command
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
    TIMEDOUT,

    FUCK_YOU
};

////////////////////////////////////////////////////
/// USER STATES
////////////////////////////////////////////////////

/// Friend relationship status
enum THORQ_RELATIONSHIP_STATUS : std::uint8_t
{
    THORQ_RELATIONSHIP_STATUS_BLOCKED, ///< Target is blocked
    THORQ_RELATIONSHIP_STATUS_NONE,    ///< No relationship status
    THORQ_RELATIONSHIP_STATUS_PENDING, ///< Outgoing friend-request
    THORQ_RELATIONSHIP_STATUS_FRIENDS, ///< Account is friended
};

/// Actions to be taken on session requested
enum THORQ_RELATIONSHIP_AUTHORITY : std::uint8_t
{
    THORQ_RELATIONSHIP_AUTHORITY_REJECT,    ///< Always reject this persons requests
    THORQ_RELATIONSHIP_AUTHORITY_SILENT,    ///< Persons requests will not prompt me
    THORQ_RELATIONSHIP_AUTHORITY_NOTIFY,    ///< Prompt me if this person requests control
    THORQ_RELATIONSHIP_AUTHORITY_ACCEPT,    ///< Accept if this person requests
    THORQ_RELATIONSHIP_AUTHORITY_EXCLUSIVE  ///< This person can invoke exclusive access to me (will kick everyone else out)
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
