using LiteDB;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace CollarControl
{
	class UserAPI
	{
		private object l_add = new object();
		private LiteDatabase _db = null;
		private LiteCollection<DbUser> _dbUsers;
		private Dictionary<Guid, User> _activeUsers;
		private List<Guid> _lockedUsers = new List<Guid>();

		public bool TryLockUser(Guid id)
		{
			lock (_lockedUsers)
			{
				if (!_lockedUsers.Contains(id))
					_lockedUsers.Add(id);
				else
					return false;
				return true;
			}
		}
		public void UnlockUser(Guid id)
		{
			lock (_lockedUsers)
				if (_lockedUsers.Contains(id))
					_lockedUsers.Remove(id);
		}

		public UserAPI(string dbPath)
		{
			_db = new LiteDatabase(dbPath);
			_dbUsers = _db.GetCollection<DbUser>("users");
			_activeUsers = new Dictionary<Guid, User>();

			var users = _dbUsers.FindAll();
			foreach (var usr in users)
				_activeUsers.Add(usr.Id, new User(usr));
		}
		public void broadcastNotification(string subject, string body)
		{
			try
			{
				ServerMessage message = new ServerMessage()
				{
					code = ServerMessage.Code.ADMIN_MSG,
					type = ServerMessage.DataType.STRING,
					requestId = Guid.Empty,
					payload = $"{Convert.ToBase64String(Encoding.Unicode.GetBytes(subject))}_{Convert.ToBase64String(Encoding.Unicode.GetBytes(body))}"
				};

				string msg = JsonConvert.SerializeObject(message);

				var users = _dbUsers.FindAll();

				foreach (User user in users)
				{
					try
					{
						user.SendMessage(msg);
					}
					catch (Exception ex)
					{
						Console.WriteLine($"Error broadcasting to {user.Username}: {ex.Message}");
					}
				}
			}
			catch (Exception ex)
			{
				Console.WriteLine($"Error starting broadcast: {ex.Message}");
			}
		}
		public void BroadcastEmail(string subject, string body)
		{
			try
			{
				var users = _dbUsers.FindAll();
				var emails = new string[users.Count()];

				int i = 0;
				foreach (DbUser user in users)
				{
					emails[i] = user.Email;
					i++;
				}

				ToolBox.SendEmail(emails, subject, body);
			}
			catch (Exception ex)
			{
				Console.WriteLine($"Error broadcastin emails: {ex.Message}");
			}
		}
		public bool TryAddUser(string username, string password, string email)
		{
			lock (_activeUsers)
			{
				email = email.ToLower();
				String lowerName = username.ToLower();
				if (_dbUsers.Exists(u => (u.Username.ToLower() == lowerName) || (u.Email == email)))
					return false;

				User user = new User(Guid.NewGuid(), username, email.ToLower(), password);
				_dbUsers.Insert(user);
				lock(_activeUsers)
					_activeUsers.Add(user.Id, user);
				return true;
			}
		}
		public void RemoveUser(string username)
		{
			lock (_activeUsers)
			{
				username = username.ToLower();

				User usr = _activeUsers.FirstOrDefault(u => u.Value.Username.ToLower() == username).Value;
				if (usr != null)
					return;

				Guid id = usr.Id;
				_dbUsers.Delete(u => u.Id == id);
				_activeUsers.Remove(id);
			}
		}
		public void RemoveUser(Guid id)
		{
			lock (_activeUsers)
			{
				if (_activeUsers.ContainsKey(id))
					return;

				_dbUsers.Delete(u => u.Id == id);
				_activeUsers.Remove(id);
			}
		}
		public bool EmailExists(string email)
		{
			email = email.ToLower();
			lock (_activeUsers)
				return _activeUsers.Any(u => u.Value.Email.ToLower() == email);
		}
		public bool UserExists(string username)
		{
			username = username.ToLower();
			lock (_activeUsers)
				return _activeUsers.Any(u => u.Value.Username.ToLower() == username);
		}
		public bool UserExists(Guid userID)
		{
			lock (_activeUsers)
				return _activeUsers.ContainsKey(userID);
		}
		public User FindUserByEmail(string email)
		{
			email = email.ToLower();
			lock (_activeUsers)
				return _activeUsers.FirstOrDefault(u => u.Value.Email.ToLower() == email).Value;
		}
		public User this[string username]
		{
			set
			{
				username = username.ToLower();
				lock (_activeUsers)
					if (value == null || !_activeUsers.Any(u => u.Value.Username.ToLower() == username))
						return;

				_dbUsers.Update(value);
				if (value.Id != Guid.Empty)
					lock (_activeUsers)
						_activeUsers[value.Id] = value;
			}
			get
			{
				username = username.ToLower();
				User user;
				lock (_activeUsers)
					user = _activeUsers.FirstOrDefault(u => u.Value.Username.ToLower() == username).Value;
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
				lock (_activeUsers)
					_activeUsers[userID] = value;
			}
			get
			{
				User user;
				lock (_activeUsers)
					user = _activeUsers[userID];
				if (user == null)
					return (User)_dbUsers.FindById(userID);
				return user;
			}
		}
	}
}
