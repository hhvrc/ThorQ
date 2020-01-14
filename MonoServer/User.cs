using LiteDB;
using System;
using System.Collections.Generic;
using System.Net.Mail;

namespace CollarControl
{
	class DbUser
	{
		[BsonId]
		public Guid Id { get; set; }
		public string Username { get; set; }
		public string Email { get; set; }
		public string PasswordHash { get; set; }
		public List<Guid> Friends { get; set; }
		public List<FriendRequest> FriendRequests { get; set; }
		public List<BlockedUser> BlockedUsers { get; set; }

		public class FriendRequest
		{
			public FriendRequest(Guid userId)
			{
				RequestId = Guid.NewGuid();
				UserID = userId;
			}
			[BsonId]
			public Guid RequestId { get; set; }
			public Guid UserID { get; set; }
		}

		public class BlockedUser
		{
			public BlockedUser(Guid userID, string username)
			{
				BlockId = Guid.NewGuid();
				UserID = userID;
				UsernameWhenBlocked = username;
			}
			[BsonId]
			public Guid BlockId { get; set; }
			public Guid UserID { get; set; }
			public string UsernameWhenBlocked { get; set; }
		}
	}

	class User : DbUser
	{
		private string passwordResetToken;
		private DateTime passwordResetExpieriDate;
		private List<Connection> _connections = new List<Connection>();
		public event Action<bool> OnIsOnlineChanged;
		public event Action<Connection, string> OnMessageReceived;

		public bool IsOnline
		{
			get
			{
				bool online = false;
				lock (_connections)
					online = _connections.Count > 0;
				return online;
			}
		}
		public bool HasConnection(Connection connection)
		{
			lock (_connections)
			{
				return _connections.Contains(connection);
			}
		}
		public void AddConnection(Connection connection)
		{
			connection.Id = this.Id;

			lock (_connections)
			{
				_connections.Add(connection);
				connection.OnMessageReceived += OnMessageReceived.Invoke;
				connection.OnClientDisconnected += RemoveConnection;
				if (_connections.Count == 1)
					OnIsOnlineChanged.Invoke(true);
			}
		}
		public void RemoveConnection(Connection connection)
		{
			connection.Id = Guid.Empty;

			lock (_connections)
			{
				if (_connections.Contains(connection))
				{
					connection.OnMessageReceived -= OnMessageReceived.Invoke;
					connection.OnClientDisconnected -= RemoveConnection;
					_connections.Remove(connection);
					if (_connections.Count == 0)
						OnIsOnlineChanged.Invoke(false);
				}
			}
		}
		public void ClearConnections()
		{
			lock (_connections)
			{
				if (_connections.Count != 0)
				{
					foreach (Connection conn in _connections)
					{
						conn.Id = Guid.Empty;
						conn.OnMessageReceived -= OnMessageReceived.Invoke;
						conn.OnClientDisconnected -= RemoveConnection;
					}
					_connections.Clear();
					OnIsOnlineChanged.Invoke(false);
				}
			}
		}
		public void SendMessage(string message)
		{
			lock (_connections)
			{
				foreach (Connection connection in _connections)
				{
					connection.SendMessage(message);
				}
			}
		}
		public void SetPassword(string password)
		{
			PasswordHash = BCrypt.Net.BCrypt.EnhancedHashPassword(password, BCrypt.Net.HashType.SHA512, 13);
		}
		public bool VerifyPassword(string password)
		{
			return BCrypt.Net.BCrypt.EnhancedVerify(password, PasswordHash, BCrypt.Net.HashType.SHA512);
		}
		public bool GeneratePasswordResetToken()
		{
			string token = ToolBox.GetUniqueToken(10);

			lock (passwordResetToken)
			{
				passwordResetToken = token;
				passwordResetExpieriDate = DateTime.UtcNow.AddMinutes(60);
			}

#if !DEBUG
			try
			{
#endif
				MailMessage mail = new MailMessage();
				SmtpClient SmtpServer = new SmtpClient("smtp.gmail.com");

				mail.From = new MailAddress("user@example.com");
				mail.To.Add(Email);
				mail.Subject = "Password Recovery";
				mail.Body = "Here is your recovery code:\n" + token;

				SmtpServer.Port = 587;
				SmtpServer.Credentials = new System.Net.NetworkCredential("user@example.com", "CollarControlPassword");
				SmtpServer.EnableSsl = true;

				SmtpServer.Send(mail);
				return true;
#if !DEBUG
			}
			catch (Exception ex)
			{
				Console.WriteLine("Exception caught while sending email: {0}", ex.Message);
			}
#endif
			return false;
		}
		public bool VerifyPasswordResetToken(string token)
		{
			lock (passwordResetToken)
			{
				return (passwordResetToken == token) && (passwordResetExpieriDate > DateTime.UtcNow);
			}
		}
	}
}
