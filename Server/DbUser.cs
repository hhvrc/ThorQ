using LiteDB;
using System;
using System.Collections.Generic;

namespace ThorQ
{
	public class DbUser
	{
		public DbUser() { }
		public DbUser(string username, string email, string password)
		{
			Id = Guid.NewGuid();
			Username = username;
			Email = email;
			PasswordHash = BCrypt.Net.BCrypt.HashPassword(password, BCrypt.Net.BCrypt.GenerateSalt(13), false, BCrypt.Net.HashType.SHA512);
			Activity = CollarLib.UserActivity.Offline;
			Status = "";
			Friends = new List<Guid>();
			Conversations = new List<Guid>();
			FriendRequests = new List<CollarLib.FriendRequest>();
			BlockedUsers = new List<CollarLib.BlockedUser>();
		}

		[BsonId]
		public Guid Id { get; set; }
		public string Username { get; set; }
		public string Email { get; set; }
		public string PasswordHash { get; set; }
		public CollarLib.UserActivity Activity { get; set; }
		public string Status { get; set; }
		public List<Guid> Friends { get; set; }
		public List<Guid> Conversations { get; set; }
		public List<CollarLib.FriendRequest> FriendRequests { get; set; }
		public List<CollarLib.BlockedUser> BlockedUsers { get; set; }
		public bool IsFriendsWith(Guid userId)
		{
			return Friends.Exists(f => f == userId);
		}
		public bool IsMemberOfConversation(Guid conversationId)
		{
			return Conversations.Exists(c => c == conversationId);
		}
		public bool HasBlocked(Guid userId)
		{
			return BlockedUsers.Exists(b => b.blockedUserId == userId);
		}
		public bool HasBlocked(String userName)
		{
			return BlockedUsers.Exists(b => b.frozenUsername == userName);
		}
		public bool VerifyPassword(string password)
		{
			return BCrypt.Net.BCrypt.Verify(password, PasswordHash, false, BCrypt.Net.HashType.SHA512);
		}
	}
}
