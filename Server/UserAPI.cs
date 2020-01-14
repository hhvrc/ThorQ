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
		private object l_add = new object();
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

			foreach (User user in _activeUsers.Values)
			{
				user.SendMessage(msg);
			}
		}
		public void BroadcastEmail(String subject, String body)
		{
			IEnumerable<DbUser> users = _dbUsers.FindAll();
			String[] emails = new String[users.Count()];

			int i = 0;
			foreach (DbUser user in users)
			{
				emails[i] = user.Email;
				i++;
			}

			ToolBox.SendEmail(emails, subject, body);
		}
		public bool TryAddUser(String username, String password, String email)
		{
			lock(l_add)
			{
				if (_dbUsers.Exists(u => (u.Username == username) || (u.Email == email)))
					return false;

				User user = new User();
				user.Id = Guid.NewGuid();
				user.Username = username;
				user.Email = email;
				user.SetPassword(password);
				_dbUsers.Insert(user);
				_activeUsers.TryAdd(user.Id, user);
				return true;
			}
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

				_dbUsers.Update(value);
				if (value.Id != Guid.Empty)
					_activeUsers.TryUpdate(value.Id, value, value);
			}
			get
			{
				username = username.ToLower();
				User user = _activeUsers.FirstOrDefault(u => u.Value.Username.ToLower() == username).Value;
				if (user == null)
					return (User)_dbUsers.FindOne(u => u.Username.ToLower() == username);
				return user;
			}
		}
		public User this[Guid userID]
		{
			set
			{
				if (value == null || !_dbUsers.Exists(u => u.Id == userID))
					return;

				_dbUsers.Update(value);
				_activeUsers.TryUpdate(userID, value, value);
			}
			get
			{
				User user = _activeUsers[userID];
				if (user == null)
					return (User)_dbUsers.FindById(userID);
				return user;
			}
		}
	}
}
