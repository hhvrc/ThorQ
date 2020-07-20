#ifndef ENUMS_H
#define ENUMS_H

namespace ThorQ {

///
/// \brief Enums that describe encryption/heartbeat (Fits in a int8)
///
enum MessageHeaderEnums {
    HEADER_HEARTBEAT  = 1 << 0,
    HEADER_ENCRYPTED  = 1 << 1,
    HEADER_RESERVED_3 = 1 << 2,
    HEADER_RESERVED_4 = 1 << 3,
    HEADER_RESERVED_5 = 1 << 4,
    HEADER_RESERVED_6 = 1 << 5,
    HEADER_RESERVED_7 = 1 << 6,
    HEADER_RESERVED_8 = 1 << 7,
};

///
/// \brief Enums that describe the message type (Fits in a int8)
///
enum MessageContentEnums {
    CONTENT_ENCRYPTED,

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

enum ClientActionFlag {
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
