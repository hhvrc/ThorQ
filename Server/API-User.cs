using LiteDB;
using Newtonsoft.Json;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace CollarControl
{
	class UserAPI
	{
		private LiteCollection<DbUser> _dbUsers;
		private ConcurrentDictionary<Guid, RuntimeUser> _activeUsers;

		public UserAPI(LiteDatabase db, Action<RuntimeUser,bool> initOnlineChaned, Action<RuntimeUser, Connection, string> initMessageReceived)
		{
			_dbUsers = db.GetCollection<DbUser>("users");
			_activeUsers = new ConcurrentDictionary<Guid, RuntimeUser>();

			var users = _dbUsers.FindAll();
			foreach (var usr in users)
			{
				RuntimeUser user = new RuntimeUser(usr);
				user.IsOnlineChanged += initOnlineChaned;
				user.MessageReceived += initMessageReceived;
				_activeUsers.TryAdd(usr.Id, user);
			}
		}

		public void broadcastNotification(string subject, string body)
		{
			try
			{
				ServerPackage message = new ServerPackage()
				{
					code = ResponseCode.ADMIN_MSG,
					type = ResponseDataType.STRING,
					requestId = Guid.Empty,
					payload = $"{Convert.ToBase64String(Encoding.Unicode.GetBytes(subject))}_{Convert.ToBase64String(Encoding.Unicode.GetBytes(body))}"
				};

				string msg = JsonConvert.SerializeObject(message);

				var users = _dbUsers.FindAll();

				foreach (RuntimeUser user in users)
				{
					try
					{
						user.SendMessage(msg);
					}
					catch (Exception ex)
					{
						Console.WriteLine($"Error broadcasting notification to {user.Username}: {ex.Message}");
					}
				}
			}
			catch (Exception ex)
			{
				Console.WriteLine($"Error starting notification broadcast: {ex.Message}");
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
				Console.WriteLine($"Error broadcasting emails: {ex.Message}");
			}
		}

		public RuntimeUser TryAdd(string username, string password, string email)
		{
			email = email.ToLower();
			string lcName = username.ToLower();
			if (_dbUsers.Exists(u => (u.Username.ToLower() == lcName) || (u.Email == email)))
				return null;

			RuntimeUser user = new RuntimeUser(username, email, password);
			_dbUsers.Insert(user);
			if (!_activeUsers.TryAdd(user.Id, user))
				Console.WriteLine($"Error adding user {user.Username} to active connections");
			return user;
		}
		public bool TryUpdate(RuntimeUser user)
		{
			if (user == null)
				return false;

			if (user.Id != Guid.Empty && _dbUsers.Exists(u => u.Id == user.Id))
			{
				_dbUsers.Update(user);
				_activeUsers[user.Id] = user;
			}
			return false;
		}
		public void TryRemove(Guid id)
		{
			lock (_activeUsers)
			{
				_activeUsers.TryRemove(id, out RuntimeUser usr);
				usr.ClearConnections();
				_dbUsers.Delete(u => u.Id == id);
			}
		}
		public bool TryRemove(string username)
		{
			// Find user
			username = username.ToLower();
			RuntimeUser usr = _activeUsers.FirstOrDefault(u => u.Value.Username.ToLower() == username).Value;
			
			// Return if not found
			if (usr == null)
				return false;

			// Remove all connections
			usr.ClearConnections();

			// Delete
			_activeUsers.TryRemove(usr.Id, out _);
			_dbUsers.Delete(u => u.Id == usr.Id);
			return true;
		}
		
		public bool EmailExists(string email)
		{
			email = email.ToLower();
			lock (_activeUsers)
				return _activeUsers.Any(u => u.Value.Email.ToLower() == email);
		}
		public bool NameExists(string username)
		{
			username = username.ToLower();
			lock (_activeUsers)
				return _activeUsers.Any(u => u.Value.Username.ToLower() == username);
		}
		public bool IdExists(Guid userID)
		{
			lock (_activeUsers)
				return _activeUsers.ContainsKey(userID);
		}

		public RuntimeUser GetById(Guid id)
		{
			RuntimeUser user;
			lock (_activeUsers)
				user = _activeUsers[id];
			if (user == null)
				return (RuntimeUser)_dbUsers.FindById(id);
			return user;
		}
		public RuntimeUser GetByName(string username)
		{
			if (username == null)
				return null;

			username = username.ToLower();
			RuntimeUser user;
			lock (_activeUsers)
				user = _activeUsers.FirstOrDefault(u => u.Value.Username.ToLower() == username).Value;
			if (user == null)
				return (RuntimeUser)_dbUsers.FindOne(u => u.Username.ToLower() == username);
			return null;
		}
		public RuntimeUser GetByEmail(string email)
		{
			if (email == null)
				return null;

			email = email.ToLower();
			lock (_activeUsers)
				return _activeUsers.FirstOrDefault(u => u.Value.Email.ToLower() == email).Value;
		}
	}
}
