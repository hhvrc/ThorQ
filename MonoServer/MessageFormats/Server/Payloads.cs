using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;

namespace CollarControl.ServerPayloads
{
	[Serializable]
	public struct Account
	{
		public string username;
		public UserActivity state;
		public string status;
		public string email;    // Only needed during registration
		public string password;

		public String Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static Account Deserialize(String str)
		{
			return JsonConvert.DeserializeObject<Account>(str);
		}
	}
	[Serializable]
	public struct BlockedUser
	{
		public Guid blockId;
		public string username; // Name of user, frozen since the time the block got applied

		public String Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static BlockedUser Deserialize(String str)
		{
			return JsonConvert.DeserializeObject<BlockedUser>(str);
		}
	}
	[Serializable]
	public struct Friend
	{
		public Guid userId;
		public string username;
		public UserActivity state;
		public string status;

		public String Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static Friend Deserialize(String str)
		{
			return JsonConvert.DeserializeObject<Friend>(str);
		}
	}
	[Serializable]
	public struct FriendMessage
	{
		public Guid senderId;
		public Guid messageId;
		public DateTime utcTime;
		public string messageContent;

		public String Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static FriendMessage Deserialize(String str)
		{
			return JsonConvert.DeserializeObject<FriendMessage>(str);
		}
	}
	[Serializable]
	public struct Conversation
	{
		public Guid id;
		public string name;
		public List<Guid> members;

		public String Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static Conversation Deserialize(String str)
		{
			return JsonConvert.DeserializeObject<Conversation>(str);
		}
	}
	[Serializable]
	public struct P2PRequest
	{
		public Guid userId;
		public Guid requestId;
		public IPAddress address;

		public String Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static P2PRequest Deserialize(String str)
		{
			return JsonConvert.DeserializeObject<P2PRequest>(str);
		}
	}
}
