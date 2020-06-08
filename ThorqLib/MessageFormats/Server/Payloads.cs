using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;

namespace CollarLib.ServerPayloads
{
	[Serializable]
	public struct AccountInstance
	{
		public AccountInstance(String username, String status, UserActivity activity)
		{
			Username = username;
			Status = status;
			Activity = activity;
			Friends = new List<Friend>();
			BlockedUsers = new List<BlockedUser>();
			FriendRequests = new List<FriendRequest>();
			Conversations = new List<Conversation>();
		}
		public string Username { get; set; }
		public string Status { get; set; }
		public UserActivity Activity { get; set; }
		public List<Friend> Friends { get; set; }
		public List<BlockedUser> BlockedUsers { get; set; }
		public List<FriendRequest> FriendRequests { get; set; }
		public List<Conversation> Conversations { get; set; }

		public String Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static AccountInstance Deserialize(String str)
		{
			return JsonConvert.DeserializeObject<AccountInstance>(str);
		}
	}
	[Serializable]
	public struct BlockedUser
	{
		public BlockedUser(Guid blockId, String frozenUsername)
		{
			BlockId = blockId;
			FrozenUsername = frozenUsername;
		}
		public Guid BlockId { get; set; }
		public string FrozenUsername { get; set; } // Name of user, frozen since the time the block got applied

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
		public Guid UserId { get; set; }
		public string Username { get; set; }
		public UserActivity Activity { get; set; }
		public string Status { get; set; }

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
		public Guid SenderId { get; set; }
		public Guid MessageId { get; set; }
		public DateTime UtcTime { get; set; }
		public string MessageContent { get; set; }

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
		public Conversation(Guid id, String name, List<Guid> members)
		{
			Id = id;
			Name = name;
			Members = members;
		}

		public Guid Id { get; set; }
		public string Name { get; set; }
		public List<Guid> Members { get; set; }

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
		public Guid UserId { get; set; }
		public Guid RequestId { get; set; }
		public IPAddress Address { get; set; }

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
