#ifndef ENUMS_H
#define ENUMS_H

typedef enum {
	THORQ_APP_SERVER,
	THORQ_APP_CLIENT,
	THORQ_APP_LINK,
} thorq_app_t;

typedef enum {
	THORQ_PAYLOAD_ID_INVALID,      ///< invalid id

	THORQ_PAYLOAD_ID_VERSION,      ///< Request updated version info

	THORQ_PAYLOAD_ID_CRYPTO,       ///< Cryptographic handshake messages
	THORQ_PAYLOAD_ID_AUTH,         ///< Client authentication (SysID/RegKey) messages

	THORQ_PAYLOAD_ID_HEARTBEAT,    ///< Message is a heartbeat, the server needs to send a heartbeat back as soon as possible when it gets this

    THORQ_PAYLOAD_ID_EVENT,        ///< Server events (user statuses)
    THORQ_PAYLOAD_ID_COMMAND,      ///< Commands like login/logout/session/requests
	THORQ_PAYLOAD_ID_COMMAND_ACK,  ///< Acknowledge for commands
    THORQ_PAYLOAD_ID_ANNOUNCEMENT, ///< Server announcement (doesnt need any sort of authentication)

	THORQ_PAYLOAD_ID_COLLAR,       ///< Collar command
} thorq_payload_id_t; ///< Message entry flags to describe the state of a message

typedef enum {
    THORQ_USER_ACTIVITY_FLAG_COLLAR_PRESENT = 1 << 0, ///< User currently has a collar connected to their pc, this will show up as a [🗲] icon next to their name
    THORQ_USER_ACTIVITY_FLAG_OPENVR_RUNNING = 1 << 1, ///< User is currently in Virtual Reality, this will show up as a [VR] icon next to their name
    THORQ_USER_ACTIVITY_FLAG_IN_SESSION     = 1 << 2, ///< User is currently in a session
    THORQ_USER_ACTIVITY_FLAG_RESERVED_4     = 1 << 3,
    THORQ_USER_ACTIVITY_FLAG_RESERVED_5     = 1 << 4,
    THORQ_USER_ACTIVITY_FLAG_RESERVED_6     = 1 << 5,
    THORQ_USER_ACTIVITY_FLAG_RESERVED_7     = 1 << 6,
    THORQ_USER_ACTIVITY_FLAG_RESERVED_8     = 1 << 7,
} thorq_user_activity_flag_t; ///< Id of a device or service that client has

typedef enum {
    THORQ_CMD_SESSION_REQUEST,
    THORQ_CMD_SESSION_ACCEPT,
    THORQ_CMD_SESSION_DENY,
    THORQ_CMD_SESSION_LEAVE,
} thorq_command_session_t;

typedef enum {
    THORQ_DISCONNECT_REASON_UNKNOWN = 0,
	THORQ_DISCONNECT_REASON_VERSION_INCOMPATIBLE,

	THORQ_DISCONNECT_REASON_CRYPT_FAILED,
	THORQ_DISCONNECT_REASON_AUTH_INVALID,

	THORQ_DISCONNECT_REASON_SHUTDOWN_CLOSED,
	THORQ_DISCONNECT_REASON_SHUTDOWN_MAINTANENCE,

	THORQ_DISCONNECT_REASON_KICKED,
	THORQ_DISCONNECT_REASON_TIMEDOUT,
} thorq_disconnect_reason_t; //

////////////////////////////////////////////////////
/// STATE MACHINES
////////////////////////////////////////////////////

typedef enum
{
	THORQ_CONNECTION_STATE_DISCONNECTED,  ///< Host is disconnected
	THORQ_CONNECTION_STATE_DISCONNECTING, ///< Host has requested that the server disconnects it gracefully
	THORQ_CONNECTION_STATE_CONNECTING,    ///< Host is connecting
	THORQ_CONNECTION_STATE_CONNECTED,     ///< Host is connected
} thorq_connection_state_t; ///< State machine for connection

typedef enum
{
    THORQ_CRYPTO_STATE_NONE = THORQ_CONNECTION_STATE_CONNECTED, ///< The cryptographic link with the other host has not been established yet
    THORQ_CRYPTO_STATE_REQUESTED,                               ///< A request has been sent to the other host, requesting to begin a handshake
    THORQ_CRYPTO_STATE_ESTABLISHING,                            ///< Hosts are now attempting to establish a shared secret
    THORQ_CRYPTO_STATE_VERIFYING,                               ///< Hosts are not trying to verify that they successfully agreed on a shared secret
    THORQ_CRYPTO_STATE_ACTIVE,                                  ///< Hosts have a shared secret and can send encrypted data between themselves
} thorq_crypto_state_t; ///< State machine for crypto

typedef enum
{
    THORQ_AUTH_STATE_NONE = THORQ_CRYPTO_STATE_ACTIVE,                           ///< Client has not been authenticated yet

    THORQ_AUTH_STATE_HWID_REQUESTING,                                            ///< Server has requested hardwareID from client
    THORQ_AUTH_STATE_HWID_CHECKING = THORQ_AUTH_STATE_HWID_REQUESTING,           ///< Client has sent SystemID to server and is awaiting a response

    THORQ_AUTH_STATE_REGKEY_REQUESTING,                                          ///< Server has requested registration key from client
    THORQ_AUTH_STATE_REGKEY_AWAITING_INPUT = THORQ_AUTH_STATE_REGKEY_REQUESTING, ///< Client is requesting user input for a registration key
    THORQ_AUTH_STATE_REGKEY_CHECKING,                                            ///< Client has semt registration key to server and is awaiting a response

    THORQ_AUTH_STATE_OK,                                                         ///< Server authenticated client, client can now access the api
} thorq_auth_state_t; ///< State machine for client authentication

typedef enum
{
    THORQ_LOGIN_STATE_LOGGEDOUT = THORQ_AUTH_STATE_OK,          ///< Client is logged out
    THORQ_LOGIN_STATE_LOGGINGOUT,                               ///< Client has requested the server to log it out gracefully
    THORQ_LOGIN_STATE_LOGGINGIN,                                ///< Client has requested to log in with a username, and waiting for the server to accept
    THORQ_LOGIN_STATE_LOGGEDIN,                                 ///< Client is logged in with a username, and is discoverable by other online users
} thorq_login_state_t; ///< State machine for login

typedef enum
{
	THORQ_SESSION_STATE_NONE,     ///< Host is not currently in a session
	THORQ_SESSION_STATE_LEAVING,  ///< Host is leaving a session
	THORQ_SESSION_STATE_DECIDING, ///< The requestee is deciding if to accept the session request
	THORQ_SESSION_STATE_JOINING,  ///< The session has been accepted and the host is waiting for the server to start it
	THORQ_SESSION_STATE_ACTIVE,   ///< Both partners are currently in a session
} thorq_session_state_t; ///< State machine for session

////////////////////////////////////////////////////
/// CLIENT SIDE
////////////////////////////////////////////////////

typedef enum {
	ACTION_Connected      = 1 << 0,  ///< [Toggle] Try to connect, and stay connected
	ACTION_ReConnect      = 1 << 1,  ///< [Signal] Try to reconnect, set this if hostname/port is changed
	ACTION_Login          = 1 << 2,  ///< [Signal] Request server to log in with a given username
	ACTION_Logout         = 1 << 3,  ///< [Signal] Request server to log out
	ACTION_ListUsers      = 1 << 4,  ///< [Signal] Request server to list online users
	ACTION_SessionRequest = 1 << 5,  ///< [Signal] Request server to start a session with a given user
	ACTION_SessionAccept  = 1 << 6,  ///< [Signal] Tell server to accept an given incoming request
	ACTION_SessionDeny    = 1 << 7,  ///< [Signal] Tell server to deny an given incoming request
	ACTION_SessionLeave   = 1 << 8,  ///< [Signal] Tell server to stop an ongoing session
    ACTION_SendRegKey     = 1 << 9,
	ACTION_RESERVED_11    = 1 << 10,
	ACTION_RESERVED_12    = 1 << 11,
	ACTION_RESERVED_13    = 1 << 12,
	ACTION_RESERVED_14    = 1 << 13,
	ACTION_RESERVED_15    = 1 << 14,
	ACTION_RESERVED_16    = 1 << 15,
	ACTION_TOGGLEACTIONS  = ACTION_Connected ///< All action flags that are meant to be toggled, and not used as signals, these flags will not be cleared after they are read
} thorq_client_action_t; ///< Flags to tell client how to behave and what to do

#endif // ENUMS_H
