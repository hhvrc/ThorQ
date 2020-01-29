using LiteDB;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;

namespace CollarControl
{
	public class RuntimeUser : DbUser
	{
		// Runtime variables
		private string passwordResetToken;
		private DateTime passwordResetExpieriDate;
		private List<Connection> _connections = new List<Connection>();

		// Events
		public event Action<RuntimeUser, bool> IsOnlineChanged;
		public event Action<RuntimeUser, Connection, string> MessageReceived;

		// Constructors
		public RuntimeUser() { }
		public RuntimeUser(string username, string email, string password, UserActivity state = UserActivity.Offline, string status = "")
		{
			base.Id = Guid.NewGuid();
			base.Username = username;
			base.Email = email;
			SetPassword(password);
			base.state = state;
			base.status = status;
			_connections = new List<Connection>();
		}
		public RuntimeUser(DbUser baseObject)
		{
			Id = baseObject.Id;
			Username = baseObject.Username;
			Email = baseObject.Email;
			PasswordHash = baseObject.PasswordHash;
			state = baseObject.state;
			status = baseObject.status;
			Friends = baseObject.Friends;
			Conversations = baseObject.Conversations;
			FriendRequests = baseObject.FriendRequests;
			BlockedUsers = baseObject.BlockedUsers;
			_connections = new List<Connection>();
		}
		~RuntimeUser()
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
			lock (_connections)
			{
				_connections.Add(connection);
				connection.OnMessageReceived += ConnectionMessageHandler;
				connection.OnClientDisconnected += RemoveConnection;
				if (_connections.Count == 1)
					IsOnlineChanged.Invoke(this, true);
			}
		}
		public void RemoveConnection(Connection connection)
		{
			lock (_connections)
			{
				if (_connections.Contains(connection))
				{
					connection.OnMessageReceived -= ConnectionMessageHandler;
					connection.OnClientDisconnected -= RemoveConnection;
					_connections.Remove(connection);
					if (_connections.Count == 0)
						IsOnlineChanged.Invoke(this, false);
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
						conn.OnMessageReceived -= ConnectionMessageHandler;
						conn.OnClientDisconnected -= RemoveConnection;
						conn.Dispose();
					}
					_connections.Clear();
					IsOnlineChanged.Invoke(this, false);
				}
			}
		}
		public bool IsFriendsWith(Guid userId)
		{
			foreach (Guid friend in Friends)
				if (friend == userId)
					return true;
			return false;
		}
		public bool IsMemberOfConversation(Guid conversationId)
		{
			foreach (Guid id in Conversations)
				if (id == conversationId)
					return true;
			return false;
		}
		public bool HasBlocked(Guid userId)
		{
			foreach (BlockedUser blocked in BlockedUsers)
				if (blocked.blockedId == userId)
					return true;
			return false;
		}
		public void SendUpdatedFriendsList()
		{
			List<ServerPayloads.Friend> friends = new List<ServerPayloads.Friend>();

			foreach (var id in Friends)
			{
				var friend = Program.userAPI.GetById(id);

				ServerPayloads.Friend f = new ServerPayloads.Friend()
				{
					userId = friend.Id,
					username = friend.Username,
					status = friend.status,
					state = friend.state,
				};

				friends.Add(f);
			}

			ServerPackage message = new ServerPackage()
			{
				code = ResponseCode.UPDATE_DATA,
				type = ResponseDataType.FRIEND_LIST,
				requestId = Guid.Empty,
				payload = JsonConvert.SerializeObject(friends),
			};

			SendMessage(message.Serialize());
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
			MessageReceived.Invoke(this, con, msg);
		}
	}
}
