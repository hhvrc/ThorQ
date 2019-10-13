using LiteDB;
using System;
using Newtonsoft.Json;
using System.Collections.Generic;
using System.Collections.Concurrent;

namespace CollarControl
{
	class UserAPI
	{
		private static LiteDatabase _db = null;
		private static LiteCollection<User> _dbUsers;
		private static LiteCollection<String> _dbEmails;
		
		public UserAPI(string dbPath)
		{
			_db = new LiteDatabase(dbPath);
			_dbUsers = _db.GetCollection<User>("users");
			_dbEmails = _db.GetCollection<String>("emails");
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

			IEnumerable<User> users = _dbUsers.FindAll();
			foreach (User user in users)
			{
				user.SendMessage(msg);
			}
		}

		public void broadcastEmail(String title, String content)
		{
			IEnumerable<String> emails = _dbEmails.FindAll();
			foreach (String email in emails)
			{
				// TODO send email
			}
		}

		public bool AddUser(String username, String password, String email)
		{
			if (UserExists(username) || EmailExists(email) || !ToolBox.IsValidEmail(email)) { return false; }

			User user = new User();
			user.Id = Guid.NewGuid();
			user.Username = username;
			user.Email = email;
			user.SetPassword(password);
			_dbUsers.Insert(user);
			return true;
		}

		public bool EmailExists(String email)
		{
			email = email.ToLower();
			return _dbEmails.Exists(e => e.ToLower() == email);
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
			return _dbUsers.FindOne(u => u.Email == email);
		}

		public User this[String username]
		{
			get
			{
				return _dbUsers.FindOne(u => u.Username == username);
			}
		}
		public User this[Guid userID]
		{
			get
			{
				return _dbUsers.FindById(userID);
			}
		}
	}
}
