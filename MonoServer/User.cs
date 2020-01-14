using LiteDB;
using System;
using System.Collections.Generic;

namespace CollarControl
{
	class DbUser
	{
		public DbUser()
		{
			Friends = new List<Guid>();
			FriendRequests = new List<FriendRequest>();
			BlockedUsers = new List<BlockedUser>();
		}

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
		// Runtime variables
		private string passwordResetToken;
		private DateTime passwordResetExpieriDate;
		private List<Connection> _connections = new List<Connection>();

		// Events
		public event Action<User, bool> OnIsOnlineChanged;
		public event Action<User, Connection, string> OnMessageReceived;

		~User()
		{
			ClearConnections();
		}

		// Functions
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
				connection.OnMessageReceived += ConnectionMessageHandler;
				connection.OnClientDisconnected += RemoveConnection;
				if (_connections.Count == 1)
					OnIsOnlineChanged.Invoke(this, true);
			}
		}
		public void RemoveConnection(Connection connection)
		{
			connection.Id = Guid.Empty;

			lock (_connections)
			{
				if (_connections.Contains(connection))
				{
					connection.OnMessageReceived -= ConnectionMessageHandler;
					connection.OnClientDisconnected -= RemoveConnection;
					_connections.Remove(connection);
					if (_connections.Count == 0)
						OnIsOnlineChanged.Invoke(this, false);
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
						conn.OnMessageReceived -= ConnectionMessageHandler;
						conn.OnClientDisconnected -= RemoveConnection;
					}
					_connections.Clear();
					OnIsOnlineChanged.Invoke(this, false);
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
			PasswordHash = BCrypt.Net.BCrypt.HashPassword(password, BCrypt.Net.BCrypt.GenerateSalt(13), false, BCrypt.Net.HashType.SHA512);
		}
		public bool VerifyPassword(string password)
		{
			return BCrypt.Net.BCrypt.Verify(password, PasswordHash, false, BCrypt.Net.HashType.SHA512);
		}
		public bool SendPasswordResetToken()
		{
			string token = ToolBox.GetUniqueToken(10);

			lock (passwordResetToken)
			{
				passwordResetToken = token;
				passwordResetExpieriDate = DateTime.UtcNow.AddMinutes(60);
			}

			try
			{
				return ToolBox.SendEmail(
					new string[] { Email },
					"Password Recovery",
					"Here is your recovery code:\n" + token
					);
			}
			catch (Exception ex)
			{
				Console.WriteLine("Exception caught while sending email: {0}", ex.Message);
			}
			return false;
		}
		public bool VerifyPasswordResetToken(string token)
		{
			lock (passwordResetToken)
			{
				return (passwordResetToken == token) && (passwordResetExpieriDate > DateTime.UtcNow);
			}
		}

		// Handlers TODO: (Relays signals to Program.cs)
		private void ConnectionMessageHandler(Connection con, string msg)
		{
			OnMessageReceived.Invoke(this, con, msg);
		}
	}
}
