using LiteDB;
using HeavenLib;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace ThorQ
{
	class UserAPI
	{
		private DbCollectionHandler<DbUser> _dbUsers;

		public UserAPI(LiteDatabase db)
		{
			_dbUsers = new DbCollectionHandler<DbUser>(db, "users");
		}

		public void BroadcastEmail(string subject, string body, Action onSuccess, Action<string> onError)
		{
			_dbUsers.AddJob((dbCollection) =>
			{
				try
				{
					var users = dbCollection.FindAll();
					var emails = new string[users.Count()];

					int i = 0;
					foreach (DbUser user in users)
					{
						emails[i] = user.Email;
						i++;
					}

					Program.emailClient.SendEmail(emails, subject, body);
					Task.Run(() => onSuccess.Invoke());
				}
				catch (Exception ex)
				{
					Task.Run(() => onError.Invoke($"Error broadcasting emails: {ex.Message}"));
				}
			});
		}

		public void GetAllUsers(Action<IEnumerable<DbUser>> onDone)
		{
			_dbUsers.AddJob((dbCollection) =>
			{
				var users = dbCollection.FindAll();
				Task.Run(() => onDone.Invoke(users));
			});
		}
		public void GetById(Guid id, Action<DbUser> onFound, Action<String> onNotFound)
		{
			_dbUsers.AddJob((dbCollection) =>
			{
				var user = dbCollection.FindById(id);

				if (user != null)
					Task.Run(() => onFound.Invoke(user));
				else
					Task.Run(() => onNotFound.Invoke("User not found"));
			});
		}
		public void GetMultipleById(List<Guid> ids, Action<List<DbUser>> onDone)
		{
			_dbUsers.AddJob((dbCollection) =>
			{
				List<DbUser> users = new List<DbUser>();
				foreach (var id in ids)
				{
					var user = dbCollection.FindById(id);
					if (user != null)
						users.Add(user);
				}
				Task.Run(() => onDone.Invoke(users));
			});
		}
		public void GetByName(string username, Action<DbUser> onFound, Action onNotFound)
		{
			_dbUsers.AddJob((dbCollection) =>
			{
				var user = dbCollection.FindOne(u => u.Username == username);

				if (user != null)
					Task.Run(() => onFound.Invoke(user));
				else
					Task.Run(() => onNotFound.Invoke());
			});
		}
		public void GetByEmail(string email, Action<DbUser> onFound, Action onNotFound)
		{
			email = email.ToLower();
			_dbUsers.AddJob((dbCollection) =>
			{
				var user = dbCollection.FindOne(u => u.Email.ToLower() == email);

				if (user != null)
					Task.Run(() => onFound.Invoke(user));
				else
					Task.Run(() => onNotFound.Invoke());
			});
		}

		// Limit userlist to friends
		public void LimitIdsToUserFriendslist(Guid userId, List<Guid> potentialFriends, Action<List<Guid>> onDone, Action<string> onError)
		{
			_dbUsers.AddJob((dbCollection) =>
			{
				DbUser thisUser = dbCollection.FindById(userId);

				if (thisUser == null)
				{
					Task.Run(() => onError.Invoke("User not found"));
					return;
				}

				for (int i = 0; i < potentialFriends.Count; ++i)
				{
					if (!thisUser.Friends.Contains(potentialFriends[i])) {
						potentialFriends.RemoveAt(i);
						--i;
					}
				}

				Task.Run(() => onDone.Invoke(potentialFriends));
			});
		}

		// User creation/deletion
		public void AddUser(string username, string password, string email, Action<DbUser> onSuccess, Action<string> onError)
		{
			email = email.ToLower();
			string lcName = username.ToLower();
			_dbUsers.AddJob((dbCollection) =>
			{
				if (dbCollection.Exists(u => (u.Username.ToLower() == lcName) || (u.Email == email)))
				{
					Task.Run(() => onError.Invoke("Username/Email already taken"));
					return;
				}

				DbUser user = new DbUser(username, email, password);
				dbCollection.Insert(user);
				Task.Run(() => onSuccess.Invoke(user));
			});

		}
		public void RemoveUser(Guid id, Action onRemoved, Action onNotFound)
		{
			_dbUsers.AddJob((dbCollection) =>
			{
				if (dbCollection.DeleteMany(u => u.Id == id) != 0)
				{
					Task.Run(() => onRemoved.Invoke());
				}
				else
				{
					Task.Run(() => onNotFound.Invoke());
				}
			});
		}
		public void RemoveUser(string username, Action onRemoved, Action onNotFound)
		{
			_dbUsers.AddJob((dbCollection) =>
			{
				if (dbCollection.DeleteMany(u => u.Username == username) != 0)
				{
					Task.Run(() => onRemoved.Invoke());
				}
				else
				{
					Task.Run(() => onNotFound.Invoke());
				}
			});
		}

		// Private setter
		private void UpdateUser(ILiteCollection<DbUser> dbCollection, DbUser user, Action<DbUser> onSuccess, Action<String> onError)
		{
			if (!dbCollection.Update(user))
				Task.Run(() => onError.Invoke("Server error"));
			else
				Task.Run(() => onSuccess.Invoke(user));
		}

		// Property setters
		/// <summary>
		/// 
		/// </summary>
		/// <param name="userId"></param>
		/// <param name="newUsername"></param>
		/// <param name="onSuccess"></param>
		/// <param name="onError"></param>
		public void SetUsername(Guid userId, String newUsername, Action onSuccess, Action<String> onError)
		{
			_dbUsers.AddJob((dbCollection) =>
			{
				var user = dbCollection.FindById(userId);
				if (user == null)
				{
					Task.Run(() => onError.Invoke("User not found"));
				}
				else
				{
					user.Username = newUsername;

					if (!dbCollection.Update(user))
						Task.Run(() => onError.Invoke("Server error"));
					else
						Task.Run(() => onSuccess.Invoke());
				}
			});
		}
		/// <summary>
		/// 
		/// </summary>
		/// <param name="userId"></param>
		/// <param name="newEmail"></param>
		/// <param name="onSuccess"></param>
		/// <param name="onError"></param>
		public void SetEmail(Guid userId, String newEmail, Action onSuccess, Action<String> onError)
		{
			_dbUsers.AddJob((dbCollection) =>
			{
				var user = dbCollection.FindById(userId);
				if (user == null)
				{
					Task.Run(() => onError.Invoke("User not found"));
				}
				else
				{
					user.Email = newEmail.ToLower();

					if (!dbCollection.Update(user))
						Task.Run(() => onError.Invoke("Server error"));
					else
						Task.Run(() => onSuccess.Invoke());
				}
			});
		}
		/// <summary>
		/// 
		/// </summary>
		/// <param name="userId"></param>
		/// <param name="newPassword"></param>
		/// <param name="onSuccess"></param>
		/// <param name="onError"></param>
		public void SetPassword(Guid userId, String newPassword, Action onSuccess, Action<String> onError)
		{
			_dbUsers.AddJob((dbCollection) =>
			{
				var user = dbCollection.FindById(userId);
				if (user == null)
				{
					Task.Run(() => onError.Invoke("User not found"));
				}
				else
				{
					user.PasswordHash = BCrypt.Net.BCrypt.HashPassword(newPassword, BCrypt.Net.BCrypt.GenerateSalt(13), false, BCrypt.Net.HashType.SHA512);

					if (!dbCollection.Update(user))
						Task.Run(() => onError.Invoke("Server error"));
					else
						Task.Run(() => onSuccess.Invoke());
				}
			});
		}
		/// <summary>
		/// 
		/// </summary>
		/// <param name="userId"></param>
		/// <param name="state"></param>
		/// <param name="onSuccess"></param>
		/// <param name="onError"></param>
		public void SetOnlineState(Guid userId, CollarLib.UserActivity state, Action<DbUser> onSuccess, Action<String> onError)
		{
			_dbUsers.AddJob((dbCollection) =>
			{
				var user = dbCollection.FindById(userId);
				if (user == null)
				{
					Task.Run(() => onError.Invoke("User not found"));
				}
				else
				{
					user.Activity = state;

					UpdateUser(dbCollection, user, onSuccess, onError);
				}
			});
		}
		/// <summary>
		/// 
		/// </summary>
		/// <param name="userId"></param>
		/// <param name="newStatus"></param>
		/// <param name="onSuccess"></param>
		/// <param name="onError"></param>
		public void SetStatus(Guid userId, String newStatus, Action<DbUser> onSuccess, Action<String> onError)
		{
			_dbUsers.AddJob((dbCollection) =>
			{
				var user = dbCollection.FindById(userId);
				if (user == null)
				{
					Task.Run(() => onError.Invoke("User not found"));
				}
				else
				{
					user.Status = newStatus;

					UpdateUser(dbCollection, user, onSuccess, onError);
				}
			});
		}
		/// <summary>
		/// Adds both users to each others friendslists, removes friend-requests, and blocks
		/// </summary>
		/// <param name="user1Id"></param>
		/// <param name="user2Id"></param>
		/// <param name="onSuccess"></param>
		/// <param name="onError"></param>
		public void CreateFriendship(Guid user1Id, Guid user2Id, Action<DbUser, DbUser> onSuccess, Action<String> onError)
		{
			_dbUsers.AddJob((dbCollection) =>
			{
				var user1 = dbCollection.FindById(user1Id);
				var user2 = dbCollection.FindById(user2Id);
				if (user1 == null || user2 == null)
				{
					Task.Run(() => onError.Invoke("User not found"));
					return;
				}

				user1.BlockedUsers.RemoveAll(b => b.blockedUserId == user2Id);
				user2.BlockedUsers.RemoveAll(b => b.blockedUserId == user1Id);

				user1.FriendRequests.RemoveAll(f => f.senderId == user2Id);
				user2.FriendRequests.RemoveAll(f => f.senderId == user1Id);

				if (!user1.IsFriendsWith(user2Id))
					user1.Friends.Add(user2Id);
				if (!user2.IsFriendsWith(user1Id))
					user2.Friends.Add(user1Id);

				bool success = dbCollection.Update(user1) && dbCollection.Update(user2);

				if (success)
				{
					Task.Run(() => onSuccess.Invoke(user1, user2));
					return;
				}
				Task.Run(() => onError.Invoke("Server error"));
			});
		}
		/// <summary>
		/// 
		/// </summary>
		/// <param name="user1Id"></param>
		/// <param name="user2Id"></param>
		/// <param name="onSuccess"></param>
		/// <param name="onError"></param>
		public void RemoveFriendship(Guid user1Id, Guid user2Id, Action<DbUser, DbUser> onSuccess, Action<String> onError)
		{
			_dbUsers.AddJob((dbCollection) =>
			{
				var user1 = dbCollection.FindById(user1Id);
				var user2 = dbCollection.FindById(user2Id);
				if (user1 == null || user2 == null)
				{
					Task.Run(() => onError.Invoke("User not found"));
					return;
				}

				if (!user1.IsFriendsWith(user2Id))
				{
					Task.Run(() => onError.Invoke("Not friends"));
					return;
				}

				user1.Friends.RemoveAll(i => i == user2Id);
				user2.Friends.RemoveAll(i => i == user1Id);

				bool success = dbCollection.Update(user1) && dbCollection.Update(user2);

				if (success)
				{
					Task.Run(() => onSuccess.Invoke(user1, user2));
					return;
				}
				Task.Run(() => onError.Invoke("Server error"));
			});
		}
		/// <summary>
		/// 
		/// </summary>
		/// <param name="coversationMembers"></param>
		/// <param name="coversationId"></param>
		/// <param name="onDone"></param>
		/// <param name="onError"></param>
		public void AddConversationToUsers(List<Guid> conversationMemberIds, Guid conversationId)
		{
			_dbUsers.AddJob((dbCollection) =>
			{
				foreach (var memberId in conversationMemberIds)
				{
					DbUser member = dbCollection.FindById(memberId);
					if (member != null)
					{
						member.Conversations.Add(conversationId);
						dbCollection.Update(member);
					}
				}
			});
		}
		/// <summary>
		/// 
		/// </summary>
		/// <param name="userId"></param>
		/// <param name="conversationId"></param>
		/// <param name="onSuccess"></param>
		/// <param name="onError"></param>
		public void AddConversation(Guid userId, Guid conversationId, Action<DbUser> onSuccess, Action<String> onError)
		{
			_dbUsers.AddJob((dbCollection) =>
			{
				var user = dbCollection.FindById(userId);
				if (user == null)
				{
					Task.Run(() => onError.Invoke("User not found"));
					return;
				}

				if (!user.Conversations.Contains(conversationId))
					user.Conversations.Add(conversationId);

				UpdateUser(dbCollection, user, onSuccess, onError);
			});
		}
		/// <summary>
		/// 
		/// </summary>
		/// <param name="userId"></param>
		/// <param name="conversationId"></param>
		/// <param name="onSuccess"></param>
		/// <param name="onError"></param>
		public void RemoveConversation(Guid userId, Guid conversationId, Action<DbUser> onSuccess, Action<String> onError)
		{
			_dbUsers.AddJob((dbCollection) =>
			{
				var user = dbCollection.FindById(userId);
				if (user == null)
				{
					Task.Run(() => onError.Invoke("User not found"));
					return;
				}

				if (user.Conversations.Contains(conversationId))
					user.Conversations.RemoveAll(c => c == conversationId);

				UpdateUser(dbCollection, user, onSuccess, onError);
			});
		}
		/// <summary>
		/// 
		/// </summary>
		/// <param name="userFrom"></param>
		/// <param name="userTo"></param>
		/// <param name="onSuccess">RequestId, TargetUserId</param>
		/// <param name="onError"></param>
		public void AddFriendRequest(Guid userFrom, String userTo, Action<Guid, Guid> onSuccess, Action<String> onError)
		{
			_dbUsers.AddJob((dbCollection) =>
			{
				var user = dbCollection.FindOne(u => u.Username == userTo);
				if (user == null)
				{
					Task.Run(() => onError.Invoke("User not found"));
					return;
				}

				if (user.FriendRequests.Exists(f => f.senderId == userFrom))
				{
					Task.Run(() => onError.Invoke("Friendrequest already sent"));
					return;
				}

				if (user.BlockedUsers.Exists(b => b.blockedUserId == userFrom))
				{
					Task.Run(() => onError.Invoke("Target user blocked user"));
					return;
				}

				var req = new CollarLib.FriendRequest(userFrom);

				user.FriendRequests.Add(req);

				if (!dbCollection.Update(user))
					Task.Run(() => onError.Invoke("Server error"));
				else
					Task.Run(() => onSuccess.Invoke(req.id, user.Id));
			});
		}
		/// <summary>
		/// 
		/// </summary>
		/// <param name="userId"></param>
		/// <param name="friendRequest"></param>
		/// <param name="onSuccess"></param>
		/// <param name="onError"></param>
		public void RemoveFriendRequest(Guid userId, Guid requestId, Action<DbUser> onSuccess, Action<String> onError)
		{
			_dbUsers.AddJob((dbCollection) =>
			{
				var user = dbCollection.FindById(userId);
				if (user == null)
				{
					Task.Run(() => onError.Invoke("User not found"));
					return;
				}

				user.FriendRequests.RemoveAll(f => f.id == requestId);

				UpdateUser(dbCollection, user, onSuccess, onError);
			});
		}
		/// <summary>
		/// 
		/// </summary>
		/// <param name="userId"></param>
		/// <param name="username"></param>
		/// <param name="onSuccess"></param>
		/// <param name="onError"></param>
		public void AddBlockedUser(Guid userId, String username, Action<DbUser> onSuccess, Action<String> onError)
		{
			_dbUsers.AddJob((dbCollection) =>
			{
				var thisUser = dbCollection.FindById(userId);
				if (thisUser == null)
				{
					Task.Run(() => onError.Invoke("User not found"));
					return;
				}

				var blockedUser = new CollarLib.BlockedUser()
				{
					id = Guid.NewGuid(),
					blockedUserId = Guid.Empty,
					frozenUsername = username
				};

				var targetUser = dbCollection.FindOne(u => u.Username == username);
				if (thisUser == null)
				{
					Task.Run(() => onError.Invoke("Target user not found"));
					return;
				}

				if (thisUser.BlockedUsers.Exists(b => b.frozenUsername == username))
				{
					Task.Run(() => onError.Invoke("User already blocked"));
				}
				else
				{
					thisUser.BlockedUsers.Add(blockedUser);
					UpdateUser(dbCollection, thisUser, onSuccess, onError);
				}
			});
		}
		/// <summary>
		/// 
		/// </summary>
		/// <param name="id"></param>
		/// <param name="username"></param>
		/// <param name="onSuccess"></param>
		/// <param name="onError"></param>
		public void RemoveBlockedUser(Guid id, String username, Action<DbUser> onSuccess, Action<String> onError)
		{
			_dbUsers.AddJob((dbCollection) =>
			{
				var user = dbCollection.FindById(id);
				if (user == null)
				{
					Task.Run(() => onError.Invoke("User not found"));
				}
				else if (user.BlockedUsers.RemoveAll(b => b.frozenUsername == username) != 0)
				{
					UpdateUser(dbCollection, user, onSuccess, onError);
					return;
				}

				Task.Run(() => onError.Invoke("User not blocked"));
			});
		}
	}
}
