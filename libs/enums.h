#ifndef ENUMS_H
#define ENUMS_H

typedef enum {
	THORQ_MSG_FLAG_ENCRYPTED  = 1 << 0, ///< The following data is encrypted, it needs to get decrypted to make sense
	THORQ_MSG_FLAG_RESERVED_2 = 1 << 1,
	THORQ_MSG_FLAG_RESERVED_3 = 1 << 2,
	THORQ_MSG_FLAG_RESERVED_4 = 1 << 3,
	THORQ_MSG_FLAG_RESERVED_5 = 1 << 4,
	THORQ_MSG_FLAG_RESERVED_6 = 1 << 5,
	THORQ_MSG_FLAG_RESERVED_7 = 1 << 6,
	THORQ_MSG_FLAG_RESERVED_8 = 1 << 7
} thorq_msg_flag_t; ///< Message entry flags to describe the state of a message

typedef enum {
	THORQ_MSG_TYPE_VERSION,   ///< thorq_version_t
	THORQ_MSG_TYPE_HEARTBEAT, ///< Message is a heartbeat, the server needs to send a heartbeat back as soon as possible when it gets this
	THORQ_MSG_TYPE_CRYPTO,
	THORQ_MSG_TYPE_COLLAR,
	THORQ_MSG_TYPE_ADMIN
} thorq_msg_type_t; ///< Message entry flags to describe the state of a message

typedef enum {
	THORQ_CMD_CRYPT_REQUEST,
	THORQ_CMD_CRYPT_ESTABLISH,
	THORQ_CMD_CRYPT_VERIFY,

	THORQ_CMD_DO_LOGIN,
	THORQ_CMD_DO_LOGOUT,

	THORQ_CMD_SESSION_REQUEST,
	THORQ_CMD_SESSION_ACCEPT,
	THORQ_CMD_SESSION_DENY,
	THORQ_CMD_SESSION_LEAVE,

    THORQ_CMD_SET_USER_STATE_FLAG,

	THORQ_CMD_GET_LIST_USERS,
	THORQ_CMD_GET_VERSION_SERVER,
	THORQ_CMD_GET_VERSION_CLIENT,
	THORQ_CMD_GET_VERSION_MESSAGE,
} THORQ_CMD;

typedef enum {
    THORQ_USER_STATE_FLAG_COLLAR_PRESENT = 1 << 0, ///< User currently has a collar connected to their pc, this will show up as a [🗲] icon next to their name
    THORQ_USER_STATE_FLAG_OPENVR_RUNNING = 1 << 1, ///< User is currently in Virtual Reality, this will show up as a [VR] icon next to their name
    THORQ_USER_STATE_FLAG_RESERVED_3 = 1 << 2,
    THORQ_USER_STATE_FLAG_RESERVED_4 = 1 << 3,
    THORQ_USER_STATE_FLAG_RESERVED_5 = 1 << 4,
    THORQ_USER_STATE_FLAG_RESERVED_6 = 1 << 5,
    THORQ_USER_STATE_FLAG_RESERVED_7 = 1 << 6,
    THORQ_USER_STATE_FLAG_RESERVED_8 = 1 << 7
} THORQ_USER_STATE_FLAG; ///< Id of a device or service that client has

typedef enum {
	THORQ_CMD_ACK_OK,
	THORQ_CMD_ACK_ERR_FAIL,
	THORQ_CMD_ACK_ERR_ACCESS_DENIED,
	THORQ_CMD_ACK_ERR_NOT_SUPPORTED,
} THORQ_CMD_ACK; ///< Acknowledge of command sent from client

typedef enum {
	THORQ_NOTIFY_USER_STATE, ///< Session state, Collar state,
	THORQ_NOTIFY_USER_OFFLINE,
	THORQ_NOTIFY_USER_OFFLINE_LOS,
	THORQ_NOTIFY_USER_OFFLINE_TIMEOUT,
	THORQ_NOTIFY_SYSTEM_BROADCAST
} THORQ_NOTIFY;

typedef enum {
	CONTENT_ENCRYPTED, ///< The data youre trying to read is encrypted, decrypt it

	COLLAR_Command,

	NOTIFY_UserOnline,
	NOTIFY_UserOffline,
	NOTIFY_UserLostConnection,
	NOTIFY_UserTimedOut,
	NOTIFY_UserAvailable,
	NOTIFY_UserInSession,
	NOTIFY_SessionAccepted,
	NOTIFY_SessionDenied,
	NOTIFY_SessionEnded,
	NOTIFY_UserCollarOn,
	NOTIFY_UserCollarOff,

	ACKNOWLEDGE_OK,
	ACKNOWLEDGE_Error,
	ACKNOWLEDGE_Denied,
	ACKNOWLEDGE_Invalid,
	ACKNOWLEDGE_LoggedIn,
	ACKNOWLEDGE_LoggedOut,

	SYSTEM_Announcement
} mavlink_msg_content_enum_t; ///< Enum that describes what the message is

typedef enum {
	THORQ_COLLAR_STATE_SHOCK      = 1 << 0, ///< Activate collar shock
	THORQ_COLLAR_STATE_VIBRATE    = 1 << 1, ///< Activate collar vibration
	THORQ_COLLAR_STATE_BEEP       = 1 << 2, ///< Activate collar speaker
	THORQ_COLLAR_STATE_AUTO       = 1 << 3, ///< Auto mode
	THORQ_COLLAR_STATE_RESERVED_5 = 1 << 4,
	THORQ_COLLAR_STATE_RESERVED_6 = 1 << 5,
	THORQ_COLLAR_STATE_RESERVED_7 = 1 << 6,
	THORQ_COLLAR_STATE_PRESENT    = 1 << 7, ///< Collar is connected
} THORQ_COLLAR_STATE; ///< Collar flag to describe current user input

typedef enum {
	ACTION_WantConnected  = 1 << 0, ///< Flip state, client will continue to try to connect as long as this is set
	ACTION_ReConnect      = 1 << 1,
	ACTION_Login          = 1 << 2,
	ACTION_Logout         = 1 << 3,
	ACTION_ListUsers      = 1 << 4,
	ACTION_SessionRequest = 1 << 5,
	ACTION_SessionAccept  = 1 << 6,
	ACTION_SessionDeny    = 1 << 7,
	ACTION_SessionLeave   = 1 << 8,
	ACTION_RESERVED_10    = 1 << 9,
	ACTION_RESERVED_11    = 1 << 10,
	ACTION_RESERVED_12    = 1 << 11,
	ACTION_RESERVED_13    = 1 << 12,
	ACTION_RESERVED_14    = 1 << 13,
	ACTION_RESERVED_15    = 1 << 14,
	ACTION_RESERVED_16    = 1 << 15,
	ACTION_TOGGLEACTIONS  = ACTION_WantConnected
} thorq_client_action_t; ///< Client side action flags

typedef enum
{
    THORQ_CONNECTION_STATE_DISCONNECTED,  ///< Host is disconnected
    THORQ_CONNECTION_STATE_DISCONNECTING, ///< Host has requested that the server disconnects it gracefully
    THORQ_CONNECTION_STATE_CONNECTING,    ///< Host is connecting
    THORQ_CONNECTION_STATE_CONNECTED      ///< Host is connected
} thorq_connection_state_t; ///< State machine for connection

typedef enum
{
    THORQ_CRYPTO_STATE_NONE,         ///< The cryptographic link with the other host has not been established yet
    THORQ_CRYPTO_STATE_REQUESTING,   ///< A request has been sent to the other host, requesting to begin a handshake
    THORQ_CRYPTO_STATE_ESTABLISHING, ///< Hosts are now attempting to establish a shared secret
    THORQ_CRYPTO_STATE_VERIFYING,    ///< Hosts are not trying to verify that they successfully agreed on a shared secret
    THORQ_CRYPTO_STATE_ACTIVE        ///< Hosts have a shared secret and can send encrypted data between themselves
} thorq_crypto_state_t; ///< State machine for crypto

typedef enum
{
    THORQ_LOGIN_STATE_LOGGEDOUT,  ///< Client is logged out
    THORQ_LOGIN_STATE_LOGGINGOUT, ///< Client has requested the server to log it out gracefully
    THORQ_LOGIN_STATE_LOGGINGIN,  ///< Client has requested to log in with a username, and waiting for the server to accept
    THORQ_LOGIN_STATE_LOGGEDIN    ///< Client is logged in with a username, and is discoverable by other online users
} thorq_login_state_t; ///< State machine for login

typedef enum
{
    THORQ_SESSION_STATE_NONE,     ///< Host is not currently in a session
    THORQ_SESSION_STATE_LEAVING,  ///< Host is leaving a session
    THORQ_SESSION_STATE_DECIDING, ///< The requestee is deciding if to accept the session request
    THORQ_SESSION_STATE_JOINING,  ///< The session has been accepted and the host is waiting for the server to start it
    THORQ_SESSION_STATE_ACTIVE    ///< Both partners are currently in a session
} thorq_session_state_t; ///< State machine for session

#endif // ENUMS_H
