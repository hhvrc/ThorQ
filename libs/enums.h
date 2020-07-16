#ifndef ENUMS_H
#define ENUMS_H

namespace ThorQ {

///
/// \brief Enums that describe encryption/heartbeat (Fits in a int8)
///
enum MessageHeaderEnums {
    HEADER_HEARTBEAT,
    HEADER_CRYPT_REQUEST,
    HEADER_CRYPT_ESTABLISH,
    HEADER_CRYPT_VERIFY,
    HEADER_CRYPT_OK
};

///
/// \brief Enums that describe the message type (Fits in a int8)
///
enum MessageContentEnums {
    USER_Login = 0,
    USER_Logout,
    USER_List,

    SESSION_Request = 16,
    SESSION_Accept,
    SESSION_Deny,
    SESSION_Leave,

    ACKNOWLEDGE_OK = 32,
    ACKNOWLEDGE_Error,
    ACKNOWLEDGE_Denied,
    ACKNOWLEDGE_Invalid,

    COMMAND_Shock = 48,
    COMMAND_Vibrate,
    COMMAND_Beep,
    COMMAND_Auto,
    COMMAND_Manual,

    NOTIFY_UserOnline = 64,
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

    ADMIN_Broadcast = 112,

    FLAG_CollarConnected = 128
};

enum ClientActionFlag {
    ACTION_Connect        = 1 << 0,
    ACTION_Disconnect     = 1 << 1,
    ACTION_Login          = 1 << 2,
    ACTION_Logout         = 1 << 3,
    ACTION_SessionRequest = 1 << 4,
    ACTION_SessionAccept  = 1 << 5,
    ACTION_SessionDeny    = 1 << 6,
    ACTION_SessionLeave   = 1 << 7,
};

enum CollarFlags {
    COLLAR_Shock      = 1 << 0,
    COLLAR_Vibrate    = 1 << 1,
    COLLAR_Beep       = 1 << 2,
    COLLAR_Auto       = 1 << 3,
    COLLAR_RESERVED_5 = 1 << 4,
    COLLAR_RESERVED_6 = 1 << 5,
    COLLAR_RESERVED_7 = 1 << 6,
    COLLAR_Present    = 1 << 7,
};

enum ClientState
{
    Disconnected,
    Disconnecting,
    Connecting,
    Connected
};

enum CryptoState
{
    None,
    Requesting,
    Establishing,
    Verifying,
    Ok
};

enum SessionState
{
    LoggedOut,
    LoggingOut,
    LoggingIn,
    LoggedIn,
    LeavingSession,
    JoiningSession,
    InSession
};
}

#endif // ENUMS_H
