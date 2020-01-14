using LiteDB;
using System;
using System.Linq;
using Newtonsoft.Json;
using System.Collections.Generic;
using System.Collections.Concurrent;

namespace CollarControl
{
	class UserAPI
	{
		private LiteDatabase _db = null;
		private LiteCollection<DbUser> _dbUsers;
		private ConcurrentDictionary<Guid, User> _activeUsers;
		
		public UserAPI(string dbPath)
		{
			_db = new LiteDatabase(dbPath);
			_dbUsers = _db.GetCollection<DbUser>("users");
			_activeUsers = new ConcurrentDictionary<Guid, User>();
		}
		public void broadcastNotification(String title, String content)
		{
			Message message = new Message()
			{
				Command = "Notification",
				Parameters = new Dictionary<string, string>()
					{
						{ "title", title },
						{ "content", content }
					}
			};
			String msg = JsonConvert.SerializeObject(message);

			foreach (KeyValuePair<Guid, User> user in _activeUsers)
			{
				user.Value.SendMessage(msg);
			}
		}
		public void broadcastEmail(String title, String content)
		{
			IEnumerable<DbUser> users = _dbUsers.FindAll();
			foreach (DbUser user in users)
			{
				// user.Email
				// TODO send email
			}
		}
		public void AddUser(String username, String password, String email)
		{
			DbUser user = new DbUser();
			user.Id = Guid.NewGuid();
			user.Username = username;
			user.Email = email;
			user.PasswordHash = password;
			_dbUsers.Insert(user);
		}
		public bool EmailExists(String email)
		{
			email = email.ToLower();
			return _dbUsers.Exists(e => e.Email.ToLower() == email);
		}
		public bool UserExists(String username)
		{
			username = username.ToLower();
			return _dbUsers.Exists(u => u.Username.ToLower() == username);
		}
		public bool UserExists(Guid userID)
		{
			return _dbUsers.FindById(userID) != null;
		}
		public User FindUserByEmail(String email)
		{
			email = email.ToLower();
			return this[_dbUsers.FindOne(u => u.Email.ToLower() == email)?.Id??Guid.Empty];
		}
		public User this[String username]
		{
			set
			{
				username = username.ToLower();
				if (value == null || !_dbUsers.Exists(u => u.Username.ToLower() == username))
					return;

				// Update activeusers
				_dbUsers.Update(value);
			}
			get
			{
				username = username.ToLower();
				return _activeUsers.FirstOrDefault(u => u.Value.Username.ToLower() == username).Value;
			}
		}
		public User this[Guid userID]
		{
			set
			{
				if (value == null || _dbUsers.FindById(userID) == null)
					return;

				_dbUsers.Update(value);
			}
			get
			{
				return _activeUsers[userID];
			}
		}
	}
}
