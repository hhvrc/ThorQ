using CollarLib;
using CollarLib.ServerPayloads;
using System;
using System.Collections.Generic;
using System.Net;

namespace ThorQ
{
	public class Instance
	{
		public string username { get; set; }
		public string status { get; set; }
		public UserActivity activity { get; set; }
		public List<Friend> friends { get; set; } = new List<Friend>();
		public List<CollarLib.BlockedUser> blockedUsers { get; set; } = new List<CollarLib.BlockedUser>();
		public List<FriendRequest> friendRequests { get; set; } = new List<FriendRequest>();
		public List<CollarLib.Conversation> conversations { get; set; } = new List<CollarLib.Conversation>();
		public IPAddress activeP2PConnection { get; set; } = null;
	}
}
