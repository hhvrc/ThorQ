using CollarLib;
using CollarLib.ServerPayloads;
using System;
using System.Collections.Generic;
using System.Net;

namespace ThorQ
{
	public class Instance
	{
		public Guid id;
		public string name;
		public string status;
		public UserActivity activity;
		public List<Friend> friends = new List<Friend>();
		public List<CollarLib.BlockedUser> blockedUsers = new List<CollarLib.BlockedUser>();
		public List<FriendRequest> friendRequests = new List<FriendRequest>();
		public List<CollarLib.Conversation> conversations = new List<CollarLib.Conversation>();
		public IPAddress activeP2PConnection = null;
	}
}
