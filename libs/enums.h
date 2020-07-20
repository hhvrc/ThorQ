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
    THORQ_MSG_FLAG_DELETE_DIS = 1 << 7, ///< Deletes all online clients from their computers
} thorq_msg_flag_t; ///< Message entry flags to describe the state of a message

typedef enum {
    THORQ_MSG_TYPE_VERSION,   ///< thorq_version_t
    THORQ_MSG_TYPE_HEARTBEAT, ///< Message is a heartbeat, the server needs to send a heartbeat back as soon as possible when it gets this
    THORQ_MSG_TYPE_CRYPTO,
    THORQ_MSG_TYPE_COLLAR,
    THORQ_MSG_TYPE_ADMIN
} thorq_msg_type_t; ///< Message entry flags to describe the state of a message

typedef enum {
    CONTENT_ENCRYPTED, ///< The data youre trying to read is encrypted, decrypt it

    CRYPT_REQUEST,
    CRYPT_ESTABLISH,
    CRYPT_VERIFY,
    CRYPT_OK,

    USER_Login,
    USER_Logout,
    USER_List,

    SESSION_Request,
    SESSION_Accept,
    SESSION_Deny,
    SESSION_Leave,

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

    ADMIN_Broadcast,

    FLAG_CollarConnected = 128
} mavlink_msg_content_enum_t; ///< Enum that describes what the message is

typedef enum {
    COLLAR_Shock      = 1 << 0, ///< Activate collar shock
    COLLAR_Vibrate    = 1 << 1, ///< Activate collar vibration
    COLLAR_Beep       = 1 << 2, ///< Activate collar speaker
    COLLAR_Auto       = 1 << 3, ///< Auto mode
    COLLAR_RESERVED_5 = 1 << 4,
    COLLAR_RESERVED_6 = 1 << 5,
    COLLAR_RESERVED_7 = 1 << 6,
    COLLAR_Present    = 1 << 7, ///< Collar is connected
} thorq_collar_flag_t; ///< Collar flag to describe current user input

typedef enum {
    ACTION_Connect        = 1 << 0,
    ACTION_Disconnect     = 1 << 1,
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
} thorq_client_action_t; ///< Client side action flags

typedef enum
{
    THORQ_CONNECTION_STATE_NONE,
    THORQ_CONNECTION_STATE_DISCONNECTING,
    THORQ_CONNECTION_STATE_CONNECTING,
    THORQ_CONNECTION_STATE_CONNECTED
} thorq_connection_state_t; ///< State machine for client

typedef enum
{
    THORQ_CRYPTO_STATE_NONE,
    THORQ_CRYPTO_STATE_REQUESTING,
    THORQ_CRYPTO_STATE_ESTABLISHING,
    THORQ_CRYPTO_STATE_VERIFYING,
    THORQ_CRYPTO_STATE_ACTIVE
} thorq_crypto_state_t; ///< State machine for cryptographic agreement with endpoint

typedef enum
{
    THORQ_LOGIN_STATE_NONE,
    THORQ_LOGIN_STATE_LOGGINGOUT,
    THORQ_LOGIN_STATE_LOGGINGIN,
    THORQ_LOGIN_STATE_LOGGEDIN
} thorq_login_state_t; ///< State machine for login and session

typedef enum
{
    THORQ_SESSION_STATE_NONE,
    THORQ_SESSION_STATE_LEAVING,
    THORQ_SESSION_STATE_JOINING,
    THORQ_SESSION_STATE_ACTIVE
} thorq_session_state_t; ///< State machine for login and session

#endif // ENUMS_H
