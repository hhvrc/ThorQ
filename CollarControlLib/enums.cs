using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CollarLib
{
	public enum UserActivity
	{
		Offline,
		Inactive,
		DoNotDisturb,
		Online,
	}
	public enum RequestType
	{
		Account,       // [GET][---][POST][DELETE][------][----]# GET=>Login POST=>Create
		Recovery,      // [---][---][POST][------][------][----]# POST=>Request
		Username,      // [GET][SET][----][------][------][----]# POST=>SendResetToken/Reset
		Email,         // [GET][SET][----][------][------][----]#
		Password,      // [---][---][POST][------][------][----]#
		BlockedUsers,  // [GET][---][POST][DELETE][------][----] POST=>Block DELETE=>Unblock
		FriendRequest, // [GET][---][POST][------][ACCEPT][DENY] POST=>Friend
		Friends,       // [GET][---][----][DELETE][------][----]
		Conversation,  // [GET][---][POST][DELETE][------][----] POST=>NewConversation DELETE=>LeaveConversation
		Message,       // [GET][SET][POST][DELETE][------][----] 
		P2PRequest,    // [---][---][POST][------][ACCEPT][DENY] Will expose IP-Adresses, non-anonymous
		RPC,           // [---][---][POST][------][------][----]
	}
	public enum RequestMethod
	{
		GET,
		SET,

		POST,
		DELETE,

		ACCEPT,
		DENY,
	}
	public enum ResponseCode
	{
		OK, // AYYYY everything ok
		NOPE, // Soft error
		ERROR,  // Hard error
		ACCEPTED,

		CREATED, // Thingy got created
		DELETED, // Thingy got deleted

		FORBIDDEN, // This method on this request is invalid
		UNAUTHORIZED, // User doesnt have the authorization to do this request

		INVALID_PARAMS, // Parameters missing/invalid
		INVALID_REQUEST, // This request is invalid

		UPDATE_DATA, // Received non-requested data from friend
		ADMIN_MSG, // Received non-requested data from admin
	}
	public enum ResponseType
	{
		NULL,
		STRING,

		USERNAME,
		EMAIL,
		PASSWORD,

		RPC, // RPC command
		P2PR, // Peer2Peer connection request

		ACCOUNT, // Account

		BLOCKED_USER, // BlockedUser
		BLOCKED_USER_LIST, // BlockedUser[]

		FRIEND, // Friend
		FRIEND_LIST, // Friend[]

		FRIEND_REQUEST, // FriendRequest
		FRIEND_REQUEST_LIST, // FriendRequest[]

		MESSAGE, // FriendMessage
		MESSAGE_LIST, // FriendMessage[]

		CONVERSATION, // FriendMessage
		CONVERSATION_LIST, // FriendMessage[]
	}
}
