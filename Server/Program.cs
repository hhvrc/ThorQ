using System;
using LiteDB;
using Newtonsoft.Json;
using System;
using System.Threading;
using System.Linq;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.Linq;

// TODO: DDOS/SPAM Protection

namespace CollarControl
{
	class Program
	{
		/// <summary>
		/// ID, Userdata
		/// </summary>
		public static ConcurrentDictionary<Guid, ActiveUser> _activeUsers = new ConcurrentDictionary<Guid, ActiveUser>();

		static LiteDatabase _db = null;
		static LiteCollection<User> _dbUsers;

		static void Main(String[] args)
		{
			Console.WriteLine(System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().CodeBase));

			_db = new LiteDatabase(@"D:\MyData.db");
			_dbUsers = _db.GetCollection<User>("users");

			Host host = new Host();
			host.OnClientConnected += ConnectionHandler;

			host.StartListening(10235);
		}

		static void ConnectionHandler(Connection client)
		{
			try
			{
				client.Authenticate();
			}
			catch (Exception ex)
			{
				Console.WriteLine("Could not authenticate: {0}", ex.Message);
			}
			Console.WriteLine("Authenticated!");

			client.OnMessageReceived += MessageHandler;

			client.StartListening();
		}

		static void MessageHandler(Connection client, String message)
		{
			Message msg;
			try
			{
				msg = JsonConvert.DeserializeObject<Message>(message);
			}
			catch (Exception ex)
			{
				Console.WriteLine("Could not deserialize message: {0}", ex.Message);
				return;
			}

			switch (msg.Command)
			{
				case "login":
					LoginHandler(client, msg);
					break;
				case "register":
					RegistrationHandler(client, msg);
					break;
				case "recover":
					RecoveryHandler(client, msg);
					break;
				default:
					break;
			}
		}

		static void LoginHandler(Connection client, Message msg)
		{
			if (!msg.Parameters.ContainsKey("username") || !msg.Parameters.ContainsKey("password"))
				return;

			String username = msg.Parameters["username"];
			String password = msg.Parameters["password"];

			if (String.IsNullOrWhiteSpace(username))
			{
				//@TODO notify user of incorrect username
				return;
			}

			Console.WriteLine("Got: " + username);

			if (String.IsNullOrWhiteSpace(password))
			{
				//@TODO notify user of incorrect password
				return;
			}

			Console.WriteLine("Got: " + password);

			User result = _dbUsers.FindOne(u => u.Username == username);
			if (result != null)
			{
				if (result.PasswordHash == password) // Hash 'password' before comparison
				{
					// Add client connection to user
				}
				else
				{
					// Kick client connection
				}
			}
			else
			{
				User u = new User();
				u.Username = username;
				u.PasswordHash = password; // @TODO @URGENT hash this
				_dbUsers.Insert(u);

				// Add client connection to user
			}

			ActiveUser databaseMockUser = _activeUsers.FirstOrDefault(u => u.Value.name == username).Value;

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
			if (_activeUsers.TryGetValue(mockGuid, out ActiveUser user))
			{
				client.Id = user.id;
				user.AddConnection(client);
				Console.WriteLine("Added connection");
			}
			else
			{
				user = new ActiveUser(username, client);
				if (!_activeUsers.TryAdd(mockGuid, user)) // @TODO gekkt information from database to create object
				{
					_activeUsers.TryGetValue(mockGuid, out user);
					user.AddConnection(client);
					Console.WriteLine("Added user");
				}
			}

			client.OnClientDisconnected += user.DisconnectHandler;

			Console.WriteLine("Started listening");
		}

		static void RegistrationHandler(Connection client, Message msg)
		{

		}

		static void RecoveryHandler(Connection client, Message msg)
		{

		}
	}
}
