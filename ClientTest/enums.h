#ifndef ENUMS_H
#define ENUMS_H

enum ThorqEnums
{
	USER_Login = 0,
	USER_Logout,
	USER_List,

	SESSION_Request = 16,
	SESSION_Accept,
	SESSION_Leave,

	ACKNOWLEDGE_OK = 32,
	ACKNOWLEDGE_Error,
	ACKNOWLEDGE_Denied,
	ACKNOWLEDGE_Invalid,

	COMMAND_Beep = 48,
	COMMAND_Vibrate,
	COMMAND_Shock,
	COMMAND_Auto,

	NOTIFY_UserOnline = 64,
	NOTIFY_UserOffline,
	NOTIFY_UserLostConnection,
	NOTIFY_UserTimedOut,
	NOTIFY_UserAvailable,
	NOTIFY_UserInSession,
	NOTIFY_SessionStarted,
	NOTIFY_SessionEnded,
	NOTIFY_UserCollarOn,
	NOTIFY_UserCollarOff,

	HEARTBEAT = 112,
	ADMIN_Broadcast,

	FLAG_CollarConnected = 128,
	FLAG_SetAutoTrigger = 256, // Sets values that Auto mode should use when being triggered
};

#endif // ENUMS_H
