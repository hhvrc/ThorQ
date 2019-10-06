using BCrypt.Net;
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

			try
			{
				host.Listen(10235);
			}
			catch (Exception ex)
			{
				Console.WriteLine("Server crashed: " + ex.Message);
			}
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
			Console.WriteLine("Authenticated!"); // DEBUG

			client.OnMessageReceived += MessageHandler;

			client.StartListening();
		}

		static String Hash(String input)
		{
			return BCrypt.Net.BCrypt.EnhancedHashPassword(input, HashType.SHA512, 13);
		}

		static bool VerifyHash(String hash, String input)
		{
			return BCrypt.Net.BCrypt.EnhancedVerify(input, hash, HashType.SHA512);
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
				case "message":
					break;
				case "friendrequest":
					break;
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
				// TODO notify user of incorrect username
				return;
			}

			Console.WriteLine("Got: " + username); // DEBUG

			if (String.IsNullOrWhiteSpace(password))
			{
				// TODO notify user of incorrect password
				return;
			}

			Console.WriteLine("Got: " + password); // DEBUG

			User user = _dbUsers.FindOne(u => u.Username == username);
			if (user == null || !VerifyHash(user.PasswordHash, user.Id + password))
			{
				// TODO Reply "Incorrect Username/Password"
				Console.WriteLine("{0} entered a incorrect password!", username); // DEBUG
				return;
			}

			ActiveUser auser = new ActiveUser(username, client);
			// Add user to active users
			if (!_activeUsers.TryAdd(user.Id, auser))
			{
				if (_activeUsers.TryGetValue(user.Id, out auser))
				{
					auser.AddConnection(client);
					Console.WriteLine("User added another device!"); // DEBUG
				}
				else
				{
					Console.WriteLine("User did... uhhhhh... Huh???"); // DEBUG
																	   // NOTE uhhhhhhhhhhhhhhhhhhhhhhhhhhh
					return;
				}
			}
			else
			{
				auser.AddConnection(client);
				Console.WriteLine("User went online!"); // DEBUG
			}

			client.OnClientDisconnected += auser.DisconnectHandler;

			Console.WriteLine("Started listening"); // DEBUG
		}

		static void RegistrationHandler(Connection client, Message msg)
		{
			if (!msg.Parameters.ContainsKey("email") ||
				!msg.Parameters.ContainsKey("username") ||
				!msg.Parameters.ContainsKey("password"))
				return;

			String email = msg.Parameters["email"];
			String username = msg.Parameters["username"];
			String password = msg.Parameters["password"];

			if (String.IsNullOrWhiteSpace(email))
			{
				// TODO notify user of incorrect password
				return;
			}
			Console.WriteLine("Got: " + email); // DEBUG

			if (String.IsNullOrWhiteSpace(username))
			{
				// TODO notify user of incorrect username
				return;
			}
			Console.WriteLine("Got: " + username); // DEBUG

			if (String.IsNullOrWhiteSpace(password))
			{
				// TODO notify user of incorrect password
				return;
			}
			Console.WriteLine("Got: " + password); // DEBUG

			if (_dbUsers.Exists(u => u.Username == username))
			{
				// TODO Reply "Username taken"
				Console.WriteLine("Username taken!"); // DEBUG
				return;
			}


			// Add user to active users
			User user = new User();
			user.Id = Guid.NewGuid();
			user.Username = username;
			user.PasswordHash = Hash(user.Id + password);
			_dbUsers.Insert(user);

			ActiveUser auser = new ActiveUser(username, client);

			if (!_activeUsers.TryAdd(user.Id, auser))
			{
				Console.WriteLine("User did... uhhhhh... Huh???"); // DEBUG
																   // NOTE uhhhhhhhhhhhhhhhhhhhhhhh
				return;
			}
			Console.WriteLine("Registered");
		}

		static void RecoveryHandler(Connection client, Message msg)
		{

		}
	}
}
