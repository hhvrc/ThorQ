using LiteDB;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using HeavenLib;
using HeavenLib.Connectivity;
using CollarLib;
using System.Text;

namespace ThorQ
{
	public class RuntimeUser : IDisposable
	{
		private Guid m_id = new Guid();
		private object l_id = new object();
		public Guid Id
		{
			get
			{
				lock (l_id)
					return m_id;
			}
		}

		// Runtime variables
		private string passwordResetToken;
		private DateTime passwordResetExpieriDate;
		private List<HostConnection> _connections = new List<HostConnection>();

		// Events
		public event Action<RuntimeUser, bool> IsOnlineChanged;
		public event Action<RuntimeUser, HostConnection, string> MessageReceived;

		// Constructors
		public RuntimeUser(Guid userId) { lock (l_id) { m_id = userId; } }

		public void Dispose()
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
		public bool HasConnection(HostConnection connection)
		{
			lock (_connections)
			{
				return _connections.Contains(connection);
			}
		}
		public bool TryAddConnection(HostConnection connection)
		{
			lock (_connections)
			{
				if (_connections.Contains(connection))
					return false;

				_connections.Add(connection);
				connection.OnMessageReceived += ConnectionMessageHandler;
				connection.OnClientDisconnected += RemoveConnection;
				if (_connections.Count == 1)
					IsOnlineChanged?.Invoke(this, true);
				return true;
			}
		}
		public void RemoveConnection(HostConnection connection)
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
					foreach (HostConnection conn in _connections)
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
		public void SendFriendsList(List<CollarLib.ServerPayloads.Friend> friends)
		{
			var message = new Response()
			{
				code = ResponseCode.UPDATE_DATA,
				type = ResponseType.FRIEND_LIST,
				requestId = Guid.Empty,
				payload = JsonConvert.SerializeObject(friends),
			};

			SendMessage(message.Serialize());
		}
		public void SendMessage(string message)
		{
			lock (_connections)
			{
				foreach (HostConnection connection in _connections)
				{
					connection.SendEncrypted(Encoding.UTF8.GetBytes(message));
				}
			}
		}

		// Handlers TODO: (Relays signals to Program.cs)
		private void ConnectionMessageHandler(HostConnection con, byte[] msg)
		{
			MessageReceived.Invoke(this, con, Encoding.UTF8.GetString(msg));
		}
		public void SendPasswordResetToken(Action onSuccess, Action<String> onFailure)
		{
			string token = ToolBox.GetUniqueToken(10);

			lock (passwordResetToken)
			{
				passwordResetToken = token;
				passwordResetExpieriDate = DateTime.UtcNow.AddMinutes(60);
			}

			try
			{
				Program.userAPI.GetById(Id,(dbUser)=>
				{
					bool success = Program.emailClient.SendEmail(
					new string[] { dbUser.Email },
					"Password Recovery",
					"Here is your recovery code:\n" + token
					);
					if (success)
						onSuccess.Invoke();
					else
						onFailure.Invoke("Failed to send email");
				},onFailure);
			}
			catch (Exception ex)
			{
				onFailure.Invoke($"Exception caught while sending email: {ex.Message}");
			}
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
