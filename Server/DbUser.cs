using LiteDB;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CollarControl
{
	public class DbUser
	{
		[BsonId]
		public Guid Id;
		public string Username = "";
		public string Email = "";
		public string PasswordHash = null;
		public CollarLib.UserActivity state = CollarLib.UserActivity.Offline;
		public string status = "";
		public List<Guid> Friends = new List<Guid>();
		public List<Guid> Conversations = new List<Guid>();
		public List<CollarLib.FriendRequest> FriendRequests = new List<CollarLib.FriendRequest>();
		public List<CollarLib.BlockedUser> BlockedUsers = new List<CollarLib.BlockedUser>();
	}
}
