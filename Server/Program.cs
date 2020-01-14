using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using static CollarControl.ToolBox;

// TODO: DDOS/SPAM Protection

namespace CollarControl
{
	class Program
	{
		public static Host host = null;
		public static UserAPI userAPI = null;
		private static Random random = new Random();

		static void Main(string[] args)
		{
			Console.WriteLine(System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().CodeBase));

			host = new Host();
			userAPI = new UserAPI(@"D:\MyData.db");

			host.OnClientConnected += (Connection con) =>
				{
					Task.Run(() => ConnectionHandler(con));
				};

#if !DEBUG
			try
			{
#endif
			host.Listen(25566);
#if !DEBUG
			}
			catch (Exception ex)
			{
				Console.WriteLine("Server crashed: " + ex.Message);
			}
#endif
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

		// Functions for logged out clients
		static void ConnectionHandler(Connection client)
		{
			Console.WriteLine("[Client] New client!");

#if !DEBUG
			try
			{
#endif
			if (client.Authenticate())
			{
				Console.WriteLine("[Client] Authenticated!");
			}
			else
			{
				Console.WriteLine("[Client] Authentication failed!");
				client.StopListening();
				return;
			}

#if !DEBUG
			try
			{
#endif
			while (client.IsConnected)
			{
				Message msg = JsonConvert.DeserializeObject<Message>(client.ReceiveMessage());

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
						SimpleResponse(client, "message", "Invalid request!");
						goto end;
				}
			}
		end:
#if !DEBUG
			}
			catch (Exception ex)
			{
				Console.WriteLine("[Client] Could not authenticate: " + ex.ToString());
			}
#endif
			try { client?.StopListening(); } catch (Exception) { }
		}
		static void LoginHandler(Connection client, Message msg)
		{
			if (client.Id != Guid.Empty)
			{
				SimpleResponse(client, "message", "Already logged in!");
				return;
			}

			if (!msg.Parameters.ContainsKey("username") || !msg.Parameters.ContainsKey("password"))
			{
				SimpleResponse(client, "message", "Invalid request!");
				return;
			}

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

			user.OnMessageReceived += UserMessageHandler;
			user.OnIsOnlineChanged += UserOnlineChanged;

			if (!user.HasConnection(client))
				user.AddConnection(client);

			Console.WriteLine("[Client] Logged in"); // DEBUG
		}
		static void RegistrationHandler(Connection client, Message msg)
		{
			if (client.Id != Guid.Empty)
			{
				SimpleResponse(client, "message", "Already logged in!");
				return;
			}

			if (!msg.Parameters.ContainsKey("email") ||
				!msg.Parameters.ContainsKey("username") ||
				!msg.Parameters.ContainsKey("password"))
			{
				SimpleResponse(client, "message", "Invalid request!");
				return;
			}

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

			// Add user
			if (!userAPI.TryAddUser(username, password, email))
			{
				SimpleResponse(client, "message", "Username/Email taken");
				return;
			}

			userAPI[username].AddConnection(client);

			Console.WriteLine("[Client] Registered");
		}
		static void RecoveryHandler(Connection client, Message msg)
		{
			if (client.Id != Guid.Empty)
			{
				SimpleResponse(client, "message", "Already logged in!");
				return;
			}

			if (!msg.Parameters.ContainsKey("email") || msg.Parameters.ContainsKey("verify") || msg.Parameters.ContainsKey("newpassword"))
			{
				SimpleResponse(client, "message", "Invalid request!");
				return;
			}

			string email = msg.Parameters["email"];
			string verify = msg.Parameters["verify"];

			if (!IsValidEmail(email))
			{
				SimpleResponse(client, "message", "Recovery password sent!");
				return;
			}

			User user = userAPI.FindUserByEmail(email);
			if (user != null)
			{
				if (verify == "")
				{
					if (!user.SendPasswordResetToken())
					{
						SimpleResponse(client, "message", "Server error!");
						return;
					}
				}
				else
				{
					if (user.VerifyPasswordResetToken(verify))
					{
						user.SetPassword(msg.Parameters["newpassword"]);
						SimpleResponse(client, "message", "Password set!");
						return;
					}
					SimpleResponse(client, "message", "Code is invalid/expired!");
					return;
				}
			}

			SimpleResponse(client, "message", "Recovery password sent!");
		}

		static void UserOnlineChanged(User user, bool online)
		{
			foreach (Guid id in user.Friends)
			{
				User friend = userAPI[id];

				if (friend == null)
					continue;

				Message message = new Message()
				{
					Command = "onlinechanged",
					Parameters = new Dictionary<string, string>()
					{
						{ "username", user.Username },
						{ "isonline", online?"true":"false" }
					}
				};
				String msg = JsonConvert.SerializeObject(message);

				friend.SendMessage(msg);
			}
		}
		static void UserMessageHandler(User user, Connection client, string message)
		{
			Message msg;
#if !DEBUG
			try
			{
#endif
			msg = JsonConvert.DeserializeObject<Message>(message);
#if !DEBUG
			}
			catch (Exception ex)
			{
				Console.WriteLine("Could not deserialize message: {0}", ex.Message);
				return;
			}
#endif

			Console.WriteLine("Got command: " + msg.Command);

			switch (msg.Command)
			{
				case "message":
					DmHandler(client, msg);
					break;
				case "friendrequest":
					FriendRequestHandler(client, msg);
					break;
				case "logout":
					LogoutHandler(client, msg);
					break;
				case "setpassword":
					SetPassword(client, msg);
					break;
				case "ping":
					msg.Command = "ACK";
					msg.Parameters.Clear();
					client.SendMessage(JsonConvert.SerializeObject(msg));
					return;
				default:
					SimpleResponse(client, "message", "Invalid request!");
					return;
			}
		}
		static void DmHandler(Connection client, Message msg)
		{
			// Get parameters
			if (!msg.Parameters.ContainsKey("username") || !msg.Parameters.ContainsKey("content"))
			{
				SimpleResponse(client, "message", "Missing parameters");
				return;
			}
			string username = msg.Parameters["username"];
			string content = msg.Parameters["content"];

			User user = userAPI[username];

			if (user != null)
			{
				Message message = new Message()
				{
					Command = "dm",
					Parameters = new Dictionary<string, string>()
					{
						{ "sender", userAPI[client.Id].Username },
						{ "content", content }
					}
				};

				string str = JsonConvert.SerializeObject(message);

				user.SendMessage(str);
			}
			else
			{
				SimpleResponse(client, "message", "Invalid user");
			}
		}
		// FIXME
		static void FriendRequestHandler(Connection client, Message msg)
		{
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
			if (targetUser.BlockedUsers.Exists(u => u.UserID == targetUser.Id))
			{
				SimpleResponse(client, "message", "You are blocked by this user");
				return;
			}

			// Send friend request (and remove potential block)
			thisUser.BlockedUsers.RemoveAll(u => u.UserID == targetUser.Id);
			if (!targetUser.FriendRequests.Exists(r => r.UserID == thisUser.Id))
			{
				DbUser.FriendRequest request = new DbUser.FriendRequest(thisUser.Id);
				targetUser.FriendRequests.Add(request);

				Message message = new Message()
				{
					Command = "FriendRequest",
					Parameters = new Dictionary<string, string>()
					{
						{ "RequestId", request.RequestId.ToString() },
						{ "Username",  thisUser.Username }
					}
				};

				string str = JsonConvert.SerializeObject(message);

				targetUser.SendMessage(str);
			}

			userAPI[thisUser.Id] = thisUser;
			userAPI[targetUser.Id] = targetUser;
		}
		static void LogoutHandler(Connection client, Message msg)
		{
			if (!msg.Parameters.ContainsKey("logoutall"))
			{
				SimpleResponse(client, "message", "Invalid request!");
				return;
			}

			string logoutall = msg.Parameters["logoutall"];

			User user = userAPI[client.Id];
			if (user == null)
			{
				// Client id is not a existing userid
				SimpleResponse(client, "message", "Server error");
				return;
			}

			if (logoutall == "true")
			{
				user.ClearConnections();
			}
			else if (logoutall == "false")
			{
				user.RemoveConnection(client);
			}
			else
			{
				SimpleResponse(client, "message", "Invalid request!");
				return;
			}

			Console.WriteLine("[Client] Logged out"); // DEBUG
		}
		static void SetPassword(Connection client, Message msg)
		{
			if (!msg.Parameters.ContainsKey("oldpassword") || !msg.Parameters.ContainsKey("newpassword"))
			{
				SimpleResponse(client, "message", "Invalid request!");
				return;
			}

			User user = userAPI[client.Id];
			if (user == null)
			{
				// Client id is not a existing userid
				SimpleResponse(client, "message", "Server error");
				return;
			}

			if (!user.VerifyPassword(msg.Parameters["oldpassword"]))
			{
				SimpleResponse(client, "message", "Invalid password!");
				return;
			}

			user.SetPassword(msg.Parameters["newpassword"]);
			SimpleResponse(client, "message", "Set password!");
		}
	}
}
