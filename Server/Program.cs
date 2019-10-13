using BCrypt.Net;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using static CollarControl.ToolBox;

// TODO: DDOS/SPAM Protection

namespace CollarControl
{
	class Program
	{
		static Host host = null;
		static UserAPI userAPI = null;

		static void Main(string[] args)
		{
			Console.WriteLine(System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().CodeBase));

			host = new Host();
			userAPI = new UserAPI(@"D:\MyData.db");

			host.OnClientConnected += ConnectionHandler;
			
			try
			{
				host.Listen(5001);
			}
			catch (Exception ex)
			{
				Console.WriteLine("Server crashed: " + ex.Message);
			}
		}

		static void ConnectionHandler(Connection client)
		{
			Console.WriteLine("[Client] New client!");

			try
			{
				if (client.Authenticate())
				{
					Console.WriteLine("[Client] Authenticated!");
				}
				else
				{
					Console.WriteLine("[Client] Authentication failed!");
					// Close connection
					client.StopListening();
					return;
				}
			}
			catch (Exception ex)
			{
				Console.WriteLine("[Client] Could not authenticate: " + ex.ToString());
				// Close connection
				return;
			}

			client.OnMessageReceived += MessageHandler;

			client.StartListening();
		}

		static string Hash(string input)
		{
			return BCrypt.Net.BCrypt.EnhancedHashPassword(input, HashType.SHA512, 13);
		}

		static bool VerifyHash(string hash, string input)
		{
			return BCrypt.Net.BCrypt.EnhancedVerify(input, hash, HashType.SHA512);
		}

		static void MessageHandler(Connection client, string message)
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
					FriendRequestHandler(client, msg);
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
				case "ping":
					msg.Command = "ACK";
					msg.Parameters.Clear();
					client.SendMessage(JsonConvert.SerializeObject(msg));
					return;
				case "exit":
					client.StopListening();
					break;
				default:
					break;
			}
		}

		private static void FriendRequestHandler(Connection client, Message msg)
		{
			if (!client.IsLoggedIn)
			{
				SimpleResponse(client, "message", "Login required");
				return;
			}

			// Get parameters
			if (!msg.Parameters.ContainsKey("username") || !msg.Parameters.ContainsKey("action"))
			{
				SimpleResponse(client, "message", "Missing parameters");
				return;
			}
			string username = msg.Parameters["username"];
			string action = msg.Parameters["action"];

			// Check if action requested is valid
			if (action != "request" && action != "accept" && action != "deny" && action != "get")
			{
				SimpleResponse(client, "message", "Invalid action");
				return;
			}

			// Find this user from database
			User thisUser = userAPI[client.Id];
			if (thisUser == null)
			{
				SimpleResponse(client, "message", "You are not in the database");
				return;
			}

			// Find target user from database
			User targetUser = userAPI[username];
			if (targetUser == null)
			{
				SimpleResponse(client, "message", "User doesnt exist");
				return;
			}

			// Check if other user has blocked this user
			if (targetUser.HasBlocked(client.Id))
			{
				SimpleResponse(client, "message", "You are blocked by this user");
				return;
			}

			// Send friend request (and remove potential block)
			thisUser.Unblock(targetUser.Id);
			if (!targetUser.HasFriendRequestFromUser(thisUser.Id))
			{
				targetUser.AddFriendRequestFromUser(thisUser.Id);
			}
		}

		static void SimpleResponse(Connection client, string key, string value)
		{
			Message message = new Message()
			{
				Command = "Response",
				Parameters = new Dictionary<string, string>()
				{
					{ key, value }
				}
			};

			string msg = JsonConvert.SerializeObject(message);

			if (client != null && !string.IsNullOrEmpty(msg))
			{
				client.SendMessage(msg);
			}
		}

		static void LoginHandler(Connection client, Message msg)
		{
			if (!msg.Parameters.ContainsKey("username") || !msg.Parameters.ContainsKey("password"))
				return;

			string username = msg.Parameters["username"];
			string password = msg.Parameters["password"];

			if (string.IsNullOrWhiteSpace(username))
			{
				SimpleResponse(client, "message", "Username cant be empty!");
				return;
			}
			Console.WriteLine("Got: " + username); // DEBUG

			if (string.IsNullOrWhiteSpace(password))
			{
				SimpleResponse(client, "message", "Password cant be empty!");
				return;
			}
			Console.WriteLine("Got: " + password); // DEBUG

			User user = userAPI[username];
			if (user == null || !user.VerifyPassword(password))
			{
				SimpleResponse(client, "message", "Invalid username/password");
				return;
			}

			client.Id = user.Id;
			if (!user.AddConnection(client))
			{
				Console.WriteLine("UHHHHH"); // DEBUG
			}

			Console.WriteLine("Started listening to connection"); // DEBUG
		}

		static void RegistrationHandler(Connection client, Message msg)
		{
			if (!msg.Parameters.ContainsKey("email") ||
				!msg.Parameters.ContainsKey("username") ||
				!msg.Parameters.ContainsKey("password"))
				return;

			string email = msg.Parameters["email"];
			string username = msg.Parameters["username"];
			string password = msg.Parameters["password"];

			if (string.IsNullOrWhiteSpace(email))
			{
				SimpleResponse(client, "message", "Email cant be empty");
				return;
			}
			if (!IsValidEmail(email))
			{
				SimpleResponse(client, "message", "Email is invalid format");
				return;
			}
			Console.WriteLine("Got: " + email); // DEBUG

			if (string.IsNullOrWhiteSpace(username))
			{
				SimpleResponse(client, "message", "Username cannot be empty");
				return;
			}
			Console.WriteLine("Got: " + username); // DEBUG

			if (string.IsNullOrWhiteSpace(password))
			{
				SimpleResponse(client, "message", "Password cannot be empty");
				return;
			}
			Console.WriteLine("Got: " + password); // DEBUG

			if (userAPI.EmailExists(email))
			{
				SimpleResponse(client, "message", "Email taken");
				return;
			}

			if (userAPI.UserExists(username))
			{
				SimpleResponse(client, "message", "Username taken");
				return;
			}

			// Add user to active users
			if (!userAPI.AddUser(username, password, email))
			{
				// Does not make sense (SHOULD NOT HAPPEN)
				SimpleResponse(client, "message", "Server error");
				return;
			}

			if (!userAPI[username].AddConnection(client))
			{
				// Does not make sense (SHOULD NOT HAPPEN)
				SimpleResponse(client, "message", "Server error");
				return;
			}

			Console.WriteLine("Registered");
		}

		static void RecoveryHandler(Connection client, Message msg)
		{

		}
	}
}
