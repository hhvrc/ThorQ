#ifndef ENUMS_H
#define ENUMS_H

#include <cstdint>
#include <QMetaEnum>

enum THORQ_APP : std::uint8_t
{
	THORQ_APP_SERVER,
	THORQ_APP_CLIENT,
	THORQ_APP_LINK,
};

/// Flags to describe the payload of a message
enum THORQ_PAYLOAD_ID
{
    THORQ_PAYLOAD_ID_INVALID,      ///< invalid id

    THORQ_PAYLOAD_ID_VERSION,      ///< Request updated version info

    THORQ_PAYLOAD_ID_HEARTBEAT,    ///< Message is a heartbeat, the server needs to send a heartbeat back as soon as possible when it gets this

    THORQ_PAYLOAD_ID_CRYPTO,       ///< Cryptographic handshake messages
//    THORQ_PAYLOAD_ID_SYSTEMID,     ///< SystemID messages
//    THORQ_PAYLOAD_ID_REGKEY,       ///< Registration-key messages
    THORQ_PAYLOAD_ID_AUTH,         ///< Client authentication (SysID/RegKey) messages

    THORQ_PAYLOAD_ID_ACCOUNT,  ///< Create/Delete/Recover Accounts
    THORQ_PAYLOAD_ID_INSTANCE, ///< Login/Logout/SetState
    THORQ_PAYLOAD_ID_SESSION,  ///< Request/Accept/Deny/Leave session

    THORQ_PAYLOAD_ID_EVENT,        ///< Server events (user statuses)
    THORQ_PAYLOAD_ID_NOTIFICATION, ///< Server notification (requires the receiver to be logged in)
    THORQ_PAYLOAD_ID_ANNOUNCEMENT, ///< Server announcement (doesnt need any sort of authentication)

    THORQ_PAYLOAD_ID_COLLAR,       ///< Collar command
};

/// Acknowledgement of message sent from remote host
enum THORQ_PAYLOAD_ACK
{
    THORQ_PAYLOAD_ACK_OK,           ///< Command succeeded
    THORQ_PAYLOAD_ACK_IN_PROGRESS,  ///< Command accepted, and is in progress
    THORQ_PAYLOAD_ACK_NO_CHANGE,    ///< Command was ignored, because it didnt change anything

    THORQ_PAYLOAD_ACK_DENIED,       ///< Command was denied
    THORQ_PAYLOAD_ACK_INVALID,      ///< Command invalid
    THORQ_PAYLOAD_ACK_LOGIN_NEEDED, ///< Client has not logged in
    THORQ_PAYLOAD_ACK_UNAUTHORIZED, ///< Client has not authenticated (Crypto + Auth)
};

/// Id of a device or service that client has
enum THORQ_USER_ACTIVITY_FLAG
{
    THORQ_USER_ACTIVITY_FLAG_COLLAR_PRESENT = 1 << 0, ///< User currently has a collar connected to their pc, this will show up as a [🗲] icon next to their name
    THORQ_USER_ACTIVITY_FLAG_OPENVR_RUNNING = 1 << 1, ///< User is currently in Virtual Reality, this will show up as a [VR] icon next to their name
    THORQ_USER_ACTIVITY_FLAG_IN_SESSION     = 1 << 2, ///< User is currently in a session
    THORQ_USER_ACTIVITY_FLAG_RESERVED_4     = 1 << 3,
    THORQ_USER_ACTIVITY_FLAG_RESERVED_5     = 1 << 4,
    THORQ_USER_ACTIVITY_FLAG_RESERVED_6     = 1 << 5,
    THORQ_USER_ACTIVITY_FLAG_RESERVED_7     = 1 << 6,
    THORQ_USER_ACTIVITY_FLAG_RESERVED_8     = 1 << 7,
};

///
enum THORQ_DISCONNECT_REASON
{
    THORQ_DISCONNECT_REASON_UNKNOWN = 0,
	THORQ_DISCONNECT_REASON_VERSION_INCOMPATIBLE,

	THORQ_DISCONNECT_REASON_CRYPT_FAILED,

    THORQ_DISCONNECT_REASON_AUTH_TIMEOUT,
	THORQ_DISCONNECT_REASON_AUTH_REGKEY_INVALID,
	THORQ_DISCONNECT_REASON_AUTH_SYSTEMID_BANNED,

	THORQ_DISCONNECT_REASON_SHUTDOWN_CLOSED,
	THORQ_DISCONNECT_REASON_SHUTDOWN_MAINTANENCE,

	THORQ_DISCONNECT_REASON_KICKED,
	THORQ_DISCONNECT_REASON_TIMEDOUT,
};

////////////////////////////////////////////////////
/// USER STATES
////////////////////////////////////////////////////

/// Friend relationship status
enum THORQ_RELATIONSHIP_FRIEND
{
	THORQ_RELATIONSHIP_FRIEND_NONE,     ///< No friendship status
	THORQ_RELATIONSHIP_FRIEND_OUTGOING, ///< Incoming friend-request
	THORQ_RELATIONSHIP_FRIEND_INCOMING, ///< Outgoing friend-request
	THORQ_RELATIONSHIP_FRIEND_FRIENDS,  ///< Account is friended
};

/// Actions to be taken on session requested
enum THORQ_RELATIONSHIP_BLOCK
{
	THORQ_RELATIONSHIP_BLOCK_NONE,     ///< Both have each other un-blocked
	THORQ_RELATIONSHIP_BLOCK_BLOCKING, ///< This account is blocking the other account
	THORQ_RELATIONSHIP_BLOCK_BLOCKED,  ///< This account is being blocked by the other account
	THORQ_RELATIONSHIP_BLOCK_MUTUAL,   ///< Both accounts have each other blocked
};

/// Actions to be taken on session requested
enum THORQ_RELATIONSHIP_AUTHORITY
{
	THORQ_RELATIONSHIP_AUTHORITY_NONE,     ///< Always reject this persons requests
	THORQ_RELATIONSHIP_AUTHORITY_REQUEST,  ///< Prompt me if this person requests control
	THORQ_RELATIONSHIP_AUTHORITY_ACCEPT,   ///< Accept if this person requests, and im not in a session
	THORQ_RELATIONSHIP_AUTHORITY_OVERRIDE, ///< Accept if this person requests, even if im in another session
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

/// @enum THORQ_STATE_AUTH
/// State machine for client authentication
enum THORQ_STATE_AUTH
{
	THORQ_STATE_AUTH_NONE = THORQ_STATE_CRYPTO_ACTIVE,                           ///< Client has not been authenticated yet

	THORQ_STATE_AUTH_HWID_REQUESTING,                                            ///< Server has requested hardwareID from client
	THORQ_STATE_AUTH_HWID_CHECKING = THORQ_STATE_AUTH_HWID_REQUESTING,           ///< Client has sent SystemID to server and is awaiting a response

	THORQ_STATE_AUTH_REGKEY_REQUESTING,                                          ///< Server has requested registration key from client
	THORQ_STATE_AUTH_REGKEY_AWAITING_INPUT = THORQ_STATE_AUTH_REGKEY_REQUESTING, ///< Client is requesting user input for a registration key
	THORQ_STATE_AUTH_REGKEY_CHECKING,                                            ///< Client has semt registration key to server and is awaiting a response

	THORQ_STATE_AUTH_OK,                                                         ///< Server authenticated client, client can now access the api
};

/// State machine for login
enum THORQ_STATE_LOGIN
{
	THORQ_STATE_LOGIN_LOGGEDOUT = THORQ_STATE_AUTH_OK,          ///< Client is logged out
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
