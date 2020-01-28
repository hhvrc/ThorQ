using System;
using System.Net;

namespace CollarControl
{
	[Serializable]
	public struct ClientMessage
	{
		public enum Request
		{
			Account,       // [GET][---][POST][DELETE][------][----] {SelfAccount}   {null} GET=>Login POST=>Create
			Recovery,      // [---][---][POST][------][------][----] {Recovery}      {null} POST=>Request
			Username,      // [GET][SET][----][------][------][----] {String}        {String} POST=>SendResetToken/Reset
			Email,         // [GET][SET][----][------][------][----] {setEmail}      {String}
			Password,      // [---][---][POST][------][------][----] {setPassword}   {null}
			BlockedUsers,  // [GET][---][POST][DELETE][------][----] {BlockedUser}   {BlockedUser[]} POST=>Block DELETE=>Unblock
			FriendRequest, // [GET][---][POST][------][ACCEPT][DENY] {FriendRequest} {FriendRequest[]} POST=>Friend
			Friends,       // [GET][---][----][DELETE][------][----] {FriendAccount} {Friend[]} DELETE=>Unfriend
			FriendMessage, // [GET][---][POST][DELETE][------][----] {FriendMessage} {FriendMessage[]}
			P2PConnection, // [---][---][POST][------][ACCEPT][DENY] {P2PRequest}    {P2PRequest} Will expose IP-Adresses, non-anonymous
			FriendSendRPC, // [---][---][POST][------][------][----] {String}        {String}
		}
		public enum Method
		{
			GET,
			SET,

			POST,
			DELETE,

			ACCEPT,
			DENY,
		}

		public Guid id;
		public Request request;
		public Method method;
		public String payload;
	}

	[Serializable]
	public struct Account
	{
		public string username;
		public DbUser.Activity state;
		public string status;
		public string email;    // Only needed during registration
		public string password;
	}
	[Serializable]
	public struct Recovery
	{
		public string email;
		public string token;
		public string newPassword;
	}
	[Serializable]
	public struct SetEmail
	{
		public string newEmail;
		public string password;
	}
	[Serializable]
	public struct SetPassword
	{
		public string oldPassword;
		public string newPassword;
	}
	[Serializable]
	public struct BlockedUser
	{
		public Guid blockId;
		public string username; // Name of user, frozen since the time the block got applied
	}
	[Serializable]
	public struct Friend
	{
		public Guid userId;
		public string username;
		public DbUser.Activity state;
		public string status;
	}
	[Serializable]
	public struct FriendRequestIn
	{
		public Guid requestId;
		public string username;
	}
	[Serializable]
	public struct FriendMessage
	{
		public Guid userId;
		public Guid messageId;
		public DateTime utcTime;
		public string messageContent;
	}
	[Serializable]
	public struct P2PRequest
	{
		public Guid userId;
		public Guid requestId;
		public IPAddress address;
	}

	[Serializable]
	public struct ServerMessage
	{
		public enum Code
		{
			OK, // AYYYY everything ok
			NOPE, // Soft error
			ERROR,	// Hard error
			ACCEPTED,

			CREATED, // Thingy got created
			DELETED, // Thingy got deleted

			FORBIDDEN, // This method on this request is invalid
			UNAUTHORIZED, // User doesnt have the authorization to do this request

			INVALID_PARAMS, // Parameters missing/invalid
			INVALID_REQUEST, // This request is invalid

			FRIEND_DATA, // Received non-requested data from friend
			ADMIN_MSG, // Received non-requested data from admin
		}
		public enum DataType
		{
			NULL,
			STRING,

			RPC, // RPC command
			P2PR, // Peer2Peer connection request

			ACCOUNT, // Account

			BLOCKED_USER, // BlockedUser
			BLOCKED_USER_LIST, // BlockedUser[]

			FRIEND, // Friend
			FRIEND_LIST, // Friend[]

			FRIEND_REQUEST, // FriendRequest
			FRIEND_REQUEST_LIST, // FriendRequest[]

			FRIEND_MESSAGE, // FriendMessage
			FRIEND_MESSAGE_LIST, // FriendMessage[]
		}
		public Code code;
		public DataType type;
		public Guid requestId; // Will be id of request if message is a response
		public String payload;
	}
}
