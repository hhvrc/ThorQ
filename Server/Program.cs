using System;
using System.IO.Ports;
using System.Text;
using System.Threading;
using System.Linq;
using System.Collections.Generic;
using System.Collections.Concurrent;

// TODO: DDOS/SPAM Protection

namespace CollarControl
{
	class Program
	{
		/// <summary>
		/// ID, Userdata
		/// </summary>
		public static ConcurrentDictionary<Guid, User> _activeUsers = new ConcurrentDictionary<Guid, User>();

		static void Main(string[] args)
		{
			Console.WriteLine(System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().CodeBase));

			Host host = new Host();
			host.OnClientConnected += LoginHandler;

			host.StartListening(10235);




			//CredentialHandler handler = new CredentialHandler();

			//handler.SetCredentials("HeavenVR", "user@example.com", "password");
			//CredentialHandler.Creds creds = handler.GetCredentials();
			//handler.SetCredentials("HeavenVR", "user@example.com", "password");
			//creds = handler.GetCredentials();

			//Console.WriteLine(creds.username);
			//Console.WriteLine(creds.passwordHash);
			//Console.WriteLine(creds.mailAddress);
		}

		static void LoginHandler(ClientConnection client)
		{
			try
			{
				client.Authenticate();
			}
			catch (Exception ex)
			{
				Console.WriteLine("Could not authenticate: " + ex.Message);
			}
			Console.WriteLine("Authenticated!");

			String username = client.ReceiveMessage();

			if (String.IsNullOrWhiteSpace(username))
			{
				//@TODO Disconnect client
				return;
			}

			Console.WriteLine("Got: " + username);

			String password = client.ReceiveMessage();

			if (String.IsNullOrWhiteSpace(password))
			{
				//@TODO Disconnect client
				return;
			}

			Console.WriteLine("Got: " + password);

			//@TODO (Username <---> Password) checking

			User databaseMockUser = _activeUsers.FirstOrDefault(u => u.Value.name == username).Value;

			Guid mockGuid;
			if (databaseMockUser != null)
			{
				mockGuid = databaseMockUser.id;
			}
			else
			{
				mockGuid = Guid.NewGuid();
			}

			Console.WriteLine("Guid: " + mockGuid);

			// Add user to active users
			if (_activeUsers.TryGetValue(mockGuid, out User user))
			{
				client.Id = user.id;
				user.AddConnection(client);
				Console.WriteLine("Added connection");
			}
			else
			{
				if (!_activeUsers.TryAdd(mockGuid, new User(username, client))) // @TODO get information from database to create object
				{
					_activeUsers.TryGetValue(mockGuid, out user);
					user.AddConnection(client);
					Console.WriteLine("Added user");
				}
			}

			client.OnClientDisconnected += user.DisconnectHandler;

			client.StartListening();
			Console.WriteLine("Started listening");
		}
	}

	class User
	{
		public Guid id;
		public String name;
		private List<ClientConnection> _connections;
		public event Action<Guid, String> OnMessageReceived;

		private void HandleMessage(ClientConnection connection, String message)
		{
			connection.SendMessage(message + "Ack.");
		}

		public User(string name, ClientConnection connection)
		{
			this.name = name;
			this.id = connection.Id;
			_connections = new List<ClientConnection>();
			_connections.Add(connection);

			connection.OnMessageReceived += HandleMessage;
		}

		public void AddConnection(ClientConnection connection)
		{
			lock (_connections)
			{
				if (!_connections.Contains(connection))
				{
					_connections.Add(connection);
					connection.OnMessageReceived += HandleMessage;
				}
			}
		}

		public void RemoveConnection(ClientConnection connection)
		{
			lock (_connections)
			{
				connection.OnMessageReceived -= HandleMessage;
				_connections.Remove(connection);
			}
		}

		public void SendMessage(String message)
		{
			lock (_connections)
			{
				foreach (ClientConnection connection in _connections)
				{
					connection.SendMessage(message);
				}
			}
		}

		public void DisconnectHandler(ClientConnection connection)
		{
			bool removeUser = false;
			lock (_connections)
			{
				_connections.Remove(connection);
				if (_connections.Count == 0)
				{
					removeUser = true;
				}
			}
			if (removeUser)
			{
				Program._activeUsers.TryRemove(this.id, out _);
			}
		}
	}

	class Connection
	{
		public User user;
		public ClientConnection connection;

		public Connection(User user, ClientConnection connection)
		{
			this.user = user;
			this.connection = connection;
		}
	}
}
