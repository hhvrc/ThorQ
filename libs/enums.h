#ifndef ENUMS_H
#define ENUMS_H

namespace ThorQ {
    enum MessageEnums
	{
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

		FLAG_CollarConnected = 128,
        FLAG_SetAutoTrigger = 256, // Sets values that Auto mode should use when being triggered
    };

    enum PreEncryptionFlag
    {
        CRYPT_REQUEST   = 1 << 0,
        CRYPT_ESTABLISH = 1 << 1,
        CRYPT_VERIFY    = 1 << 2,
        CRYPT_OK        = 1 << 3,
        HEARTBEAT       = 1 << 4,
        RESERVED_6      = 1 << 5,
        RESERVED_7      = 1 << 6,
        RESERVED_8      = 1 << 7,
    };

    enum ClientActionFlag
    {
        ACTION_Connect        = 1 << 0,
        ACTION_Disconnect     = 1 << 1,
        ACTION_Login          = 1 << 2,
        ACTION_Logout         = 1 << 3,
        ACTION_SessionRequest = 1 << 4,
        ACTION_SessionAccept  = 1 << 5,
        ACTION_SessionDeny    = 1 << 6,
        ACTION_SessionLeave   = 1 << 7,
    };

    enum CollarFlags
    {
        COLLAR_Shock   = 1 << 0,
        COLLAR_Vibrate = 1 << 1,
        COLLAR_Beep    = 1 << 2,
        COLLAR_Auto    = 1 << 3,
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

    enum CommandEnums
    {
        Shock,
        Vibrate,
        Beep,
        Auto,
        Manual
    };
}

#endif // ENUMS_H
