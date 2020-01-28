using Newtonsoft.Json;
using System;
using System.IO;
using System.Text;
using System.Threading;
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
			string thisPath = ToolBox.GetExeDirectory();
			Console.WriteLine(thisPath);
#if DEBUG
			args = new string[] { "5001", "1" };
#endif

			if (args.Length != 2)
			{
				Console.WriteLine("Server.exe [port] [useIPv6?]");
				return;
			}

			if (!ushort.TryParse(args[0], out ushort port))
			{
				Console.WriteLine("Port number invalid!");
				return;
			}

			bool useIPv6 = false;
			if (args[1].ToLower() == "true" || args[1] == "1")
				useIPv6 = true;
			else if (args[1].ToLower() == "false" || args[1] == "0")
				useIPv6 = false;
			else
			{
				Console.WriteLine("Invalid boolean input!");
				return;
			}

			host = new Host();
			userAPI = new UserAPI(Path.Combine(thisPath, "MyData.db"));

			// @TODO: Init all users to allocated object, and connect signals to OnUserOnlineChanged

			host.OnClientConnected += (Connection con) =>
				{
					Task.Run(() => ConnectionHandler(con));
				};
			try
			{
				host.Listen(port, useIPv6);
			}
			catch (Exception ex)
			{
				Console.WriteLine($"Server crashed: {ex.Message}");
			}
		}

		static void SimpleClientResponse(Connection client, Guid requestId, ServerMessage.Code code, string message)
		{
			ServerMessage messageObject = new ServerMessage()
			{
				code = code,
				type = ((message == null) ? ServerMessage.DataType.NULL : ServerMessage.DataType.STRING),
				requestId = requestId,
				payload = ((message == null) ? null : Convert.ToBase64String(Encoding.Unicode.GetBytes(message))),
			};

			string jsonMessage = JsonConvert.SerializeObject(messageObject);

			if (client != null && !string.IsNullOrEmpty(jsonMessage))
			{
				client.SendMessage(jsonMessage);
			}
		}

		// Uninitialized clients
		static void ConnectionHandler(Connection client)
		{
			Console.WriteLine("[Client] New client");

			try
			{
				if (client.Authenticate())
				{
					Console.WriteLine("[Client] Authenticated");
				}
				else
				{
					Console.WriteLine("[Client] Authentication failed");
					client.StopListening();
					return;
				}
			}
			catch (Exception ex)
			{
				Console.WriteLine($"[Client] Could not authenticate: {ex.Message}");
				try { client?.StopListening(); } catch (Exception) { }
				return;
			}

			// @TODO: make array for unauthed clients
			client.OnMessageReceived += MessageHandler;
		}

		static void MessageHandler(Connection client, string str)
		{
			try
			{
				while (client.IsConnected)
				{
					ClientMessage msg = JsonConvert.DeserializeObject<ClientMessage>(str);

					switch (msg.request)
					{
						case ClientMessage.Request.Account:
							HandleRequest_Account(client, msg.method, msg.id, msg.payload);
							break;
						case ClientMessage.Request.Recovery:
							HandleRequest_Recovery(client, msg.method, msg.id, msg.payload);
							break;
						case ClientMessage.Request.Username:
							HandleRequest_Username(client, msg.method, msg.id, msg.payload);
							break;
						case ClientMessage.Request.Email:
							HandleRequest_Email(client, msg.method, msg.id, msg.payload);
							break;
						case ClientMessage.Request.Password:
							HandleRequest_Password(client, msg.method, msg.id, msg.payload);
							break;
						case ClientMessage.Request.BlockedUsers:
							HandleRequest_BlockedUsers(client, msg.method, msg.id, msg.payload);
							break;
						case ClientMessage.Request.FriendRequest:
							HandleRequest_FriendRequest(client, msg.method, msg.id, msg.payload);
							break;
						case ClientMessage.Request.Friends:
							HandleRequest_Friends(client, msg.method, msg.id, msg.payload);
							break;
						case ClientMessage.Request.FriendMessage:
							HandleRequest_FriendsMessage(client, msg.method, msg.id, msg.payload);
							break;
						case ClientMessage.Request.P2PConnection:
							HandleRequest_P2PConnection(client, msg.method, msg.id, msg.payload);
							break;
						case ClientMessage.Request.FriendSendRPC:
							HandleRequest_FriendSendRPC(client, msg.method, msg.id, msg.payload);
							break;
						default:
							SimpleClientResponse(client, msg.id, ServerMessage.Code.INVALID_REQUEST, "Not a valid request");
							break;
					}
				}
			}
			catch (Exception ex)
			{
				SimpleClientResponse(client, Guid.Empty, ServerMessage.Code.ERROR, "Invalid message");
				Console.WriteLine($"[Client] Could not receive message: {ex.Message}");
			}
		}

		static void UserOnlineChanged(User user, bool online)
		{
			Friend friendMsg = new Friend()
			{
				userId = user.Id,
				username = user.Username,
				state = (online ? user.state : DbUser.Activity.Offline),
				status = user.status
			};
			string msg = JsonConvert.SerializeObject(friendMsg);
			ServerMessage message = new ServerMessage()
			{
				code = ServerMessage.Code.FRIEND_DATA,
				type = ServerMessage.DataType.FRIEND,
				requestId = Guid.Empty,
				payload = msg
			};
			msg = JsonConvert.SerializeObject(message);

			foreach (Guid id in user.Friends)
			{
				User friend;
				friend = userAPI[id];

				if (friend != null)
					friend.SendMessage(msg);
			}
		}

		// Account requests
		static void HandleRequest_Account(Connection client, ClientMessage.Method method, Guid requestId, string payload)
		{
			Account account;
			try
			{
				account = JsonConvert.DeserializeObject<Account>(payload);
			}
			catch (Exception)
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.ERROR, "Invalid payload");
				return;
			}

			switch (method)
			{
				case ClientMessage.Method.GET:
					AccountLoginHandler(client, requestId, account);
					break;
				case ClientMessage.Method.POST:
					AccountRegistrationHandler(client, requestId, account);
					break;
				case ClientMessage.Method.DELETE:
					AccountDeletionHandler(client, requestId, account);
					break;
				default:
					SimpleClientResponse(client, requestId, ServerMessage.Code.FORBIDDEN, "Not a valid method for this request");
					break;
			}
		}
		static void AccountLoginHandler(Connection client, Guid requestId, Account account)
		{
			if (client.Id != Guid.Empty)
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.NOPE, "Already logged in");
				return;
			}

			if (string.IsNullOrWhiteSpace(account.username))
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.INVALID_PARAMS, "Username cannot be empty");
				return;
			}
			Console.WriteLine("Got: " + account.username); // DEBUG

			if (string.IsNullOrWhiteSpace(account.password))
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.INVALID_PARAMS, "Password cannot be empty");
				return;
			}
			Console.WriteLine("Got: " + account.password); // DEBUG

			while (!userAPI.TryLockUser(Guid.Empty))
				Thread.Sleep(10);
			try
			{
				User user = userAPI[account.username];
				if (user == null || !user.VerifyPassword(account.password))
				{
					SimpleClientResponse(client, requestId, ServerMessage.Code.UNAUTHORIZED, "Invalid username/password");
					return;
				}

				if (!user.HasConnection(client))
				{
					client.OnMessageReceived -= MessageHandler;
					user.AddConnection(client);
				}
			}
			catch (Exception ex)
			{
				Console.WriteLine($"Exception: {ex.Message}"); // DEBUG
				return;
			}
			finally
			{
				userAPI.UnlockUser(Guid.Empty);
			}

			SimpleClientResponse(client, requestId, ServerMessage.Code.OK, "Loggged in");
			Console.WriteLine("[Client] Logged in"); // DEBUG
		}
		static void AccountRegistrationHandler(Connection client, Guid requestId, Account account)
		{
			if (client.Id != Guid.Empty)
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.NOPE, "Already logged in");
				return;
			}

			if (string.IsNullOrWhiteSpace(account.email))
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.INVALID_PARAMS, "Email cannot be empty");
				return;
			}
			if (!IsValidEmail(account.email))
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.INVALID_PARAMS, "Email is invalid format");
				return;
			}
			Console.WriteLine("Got: " + account.email); // DEBUG

			if (string.IsNullOrWhiteSpace(account.username))
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.INVALID_PARAMS, "Username cannot be empty");
				return;
			}
			Console.WriteLine("Got: " + account.username); // DEBUG

			if (string.IsNullOrWhiteSpace(account.password))
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.INVALID_PARAMS, "Password cannot be empty");
				return;
			}
			Console.WriteLine("Got: " + account.password); // DEBUG

			while (!userAPI.TryLockUser(Guid.Empty))
				Thread.Sleep(10);
			try
			{
				if (userAPI.EmailExists(account.email))
				{
					SimpleClientResponse(client, requestId, ServerMessage.Code.NOPE, "Email taken");
					return;
				}

				if (userAPI.UserExists(account.username))
				{
					SimpleClientResponse(client, requestId, ServerMessage.Code.NOPE, "Username taken");
					return;
				}

				// Add user
				if (!userAPI.TryAddUser(account.username, account.password, account.email))
				{
					SimpleClientResponse(client, requestId, ServerMessage.Code.NOPE, "Username/Email taken");
					return;
				}

				client.OnMessageReceived -= MessageHandler;

				userAPI[account.username].AddConnection(client);
			}
			catch (Exception ex)
			{
				Console.WriteLine($"Exception: {ex.Message}"); // DEBUG
				SimpleClientResponse(client, requestId, ServerMessage.Code.ERROR, "Server error");
				return;
			}
			finally
			{
				userAPI.UnlockUser(Guid.Empty);
			}

			SimpleClientResponse(client, requestId, ServerMessage.Code.CREATED, "Account created");
			Console.WriteLine("[Client] Registered");
		}
		static void AccountDeletionHandler(Connection client, Guid requestId, Account account)
		{
			if (client.Id == Guid.Empty)
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.UNAUTHORIZED, "Not logged in");
				return;
			}

			if (string.IsNullOrWhiteSpace(account.username))
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.INVALID_PARAMS, "Username cannot be empty");
				return;
			}
			Console.WriteLine("Got: " + account.username); // DEBUG

			if (string.IsNullOrWhiteSpace(account.password))
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.INVALID_PARAMS, "Password cannot be empty");
				return;
			}
			Console.WriteLine("Got: " + account.password); // DEBUG

			while (!userAPI.TryLockUser(Guid.Empty))
				Thread.Sleep(5);
			try
			{
				User user = userAPI[account.username];
				if (user == null || !user.VerifyPassword(account.password))
				{
					SimpleClientResponse(client, requestId, ServerMessage.Code.UNAUTHORIZED, "Invalid username/password");
					return;
				}

				user.RemoveConnection(client);
				userAPI.RemoveUser(user.Id);
			}
			catch (Exception ex)
			{
				Console.WriteLine($"Exception: {ex.Message}"); // DEBUG
				return;
			}
			finally
			{
				userAPI.UnlockUser(Guid.Empty);
			}

			client.OnMessageReceived += MessageHandler;

			SimpleClientResponse(client, requestId, ServerMessage.Code.DELETED, "Account deleted");
			Console.WriteLine("[Client] Deleted account"); // DEBUG
		}

		// Account recovery
		static void HandleRequest_Recovery(Connection client, ClientMessage.Method method, Guid requestId, string payload)
		{
			if (method != ClientMessage.Method.POST)
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.FORBIDDEN, "Not a valid method for this request");
				return;
			}

			Recovery recovery;
			try
			{
				recovery = JsonConvert.DeserializeObject<Recovery>(payload);
			}
			catch (Exception)
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.ERROR, "Invalid payload");
				return;
			}

			if (client.Id != Guid.Empty)
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.NOPE, "Already logged in!");
				return;
			}

			if (!IsValidEmail(recovery.email))
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.OK, "Recovery password sent!");
				return;
			}

			while (!userAPI.TryLockUser(Guid.Empty))
				Thread.Sleep(10);
			try
			{
				User user = userAPI.FindUserByEmail(recovery.email);
				if (user != null)
				{
					if (recovery.token == "")
					{
						if (!user.SendPasswordResetToken())
						{
							SimpleClientResponse(client, requestId, ServerMessage.Code.ERROR, "Failed to send email!");
							return;
						}
					}
					else
					{
						if (user.VerifyPasswordResetToken(recovery.token))
						{
							user.SetPassword(recovery.newPassword);
							SimpleClientResponse(client, requestId, ServerMessage.Code.OK, "Password set!");
							return;
						}
						SimpleClientResponse(client, requestId, ServerMessage.Code.NOPE, "Code is invalid/expired!");
						return;
					}
				}
			}
			catch (Exception ex)
			{
				Console.WriteLine($"Exception: {ex.Message}"); // DEBUG
				return;
			}
			finally
			{
				userAPI.UnlockUser(Guid.Empty);
			}

			SimpleClientResponse(client, requestId, ServerMessage.Code.OK, "Recovery password sent!");
		}

		// Account properties requests
		static void HandleRequest_Username(Connection client, ClientMessage.Method method, Guid requestId, string payload)
		{
			// Check if user is logged in
			if (client.Id == Guid.Empty)
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.UNAUTHORIZED, "Not logged in");
				return;
			}

			while (!userAPI.TryLockUser(Guid.Empty))
				Thread.Sleep(10);
			try
			{
				// Get user
				User user = userAPI[client.Id];
				if (user == null)
				{
					SimpleClientResponse(client, requestId, ServerMessage.Code.ERROR, "Server error");
					return;
				}

				// Get/Set username
				switch (method)
				{
					case ClientMessage.Method.GET:
						SimpleClientResponse(client, requestId, ServerMessage.Code.OK, user.Username);
						break;
					case ClientMessage.Method.SET:
						if (string.IsNullOrWhiteSpace(payload))
						{
							SimpleClientResponse(client, requestId, ServerMessage.Code.INVALID_PARAMS, "Username cannot be empty");
							return;
						}
						Console.WriteLine("Got: " + payload); // DEBUG
						user.Username = payload;
						userAPI[client.Id] = user;
						SimpleClientResponse(client, requestId, ServerMessage.Code.OK, user.Username);
						break;
					default:
						SimpleClientResponse(client, requestId, ServerMessage.Code.FORBIDDEN, "Not a valid method for this request");
						break;
				}
			}
			catch (Exception ex)
			{
				Console.WriteLine($"Exception: {ex.Message}"); // DEBUG
				return;
			}
			finally
			{
				userAPI.UnlockUser(Guid.Empty);
			}
		}
		static void HandleRequest_Email(Connection client, ClientMessage.Method method, Guid requestId, string payload)
		{
			// Check if user is logged in
			if (client.Id == Guid.Empty)
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.UNAUTHORIZED, "Not logged in");
				return;
			}

			// Get user
			User user = userAPI[client.Id];
			if (user == null)
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.ERROR, "Server error");
				return;
			}

			// Get/Set username
			switch (method)
			{
				case ClientMessage.Method.GET:
					SimpleClientResponse(client, requestId, ServerMessage.Code.OK, user.Email);
					break;
				case ClientMessage.Method.SET:
					SetEmail recovery;
					try
					{
						recovery = JsonConvert.DeserializeObject<SetEmail>(payload);
					}
					catch (Exception)
					{
						SimpleClientResponse(client, requestId, ServerMessage.Code.ERROR, "Invalid payload");
						return;
					}
					if (user.VerifyPassword(recovery.password))
					{
						Console.WriteLine("Got: " + recovery.newEmail); // DEBUG
						user.Email = recovery.newEmail;
						userAPI[client.Id] = user;
						SimpleClientResponse(client, requestId, ServerMessage.Code.OK, user.Email);
					}
					SimpleClientResponse(client, requestId, ServerMessage.Code.UNAUTHORIZED, "Invalid password");
					break;
				default:
					SimpleClientResponse(client, requestId, ServerMessage.Code.FORBIDDEN, "Not a valid method for this request");
					break;
			}
		}
		static void HandleRequest_Password(Connection client, ClientMessage.Method method, Guid requestId, string payload)
		{
			// Check if user is logged in
			if (client.Id == Guid.Empty)
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.UNAUTHORIZED, "Not logged in");
				return;
			}

			// Get user
			User user = userAPI[client.Id];
			if (user == null)
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.ERROR, "Server error");
				return;
			}

			// Get/Set username
			switch (method)
			{
				case ClientMessage.Method.POST:
					SetPassword recovery;
					try
					{
						recovery = JsonConvert.DeserializeObject<SetPassword>(payload);
					}
					catch (Exception)
					{
						SimpleClientResponse(client, requestId, ServerMessage.Code.ERROR, "Invalid payload");
						return;
					}
					if (user.VerifyPassword(recovery.oldPassword))
					{
						Console.WriteLine("Got: " + recovery.newPassword); // DEBUG
						user.SetPassword(recovery.newPassword);
						userAPI[client.Id] = user;
						SimpleClientResponse(client, requestId, ServerMessage.Code.OK, "Password set!");
					}
					SimpleClientResponse(client, requestId, ServerMessage.Code.UNAUTHORIZED, "Invalid password");
					break;
				default:
					SimpleClientResponse(client, requestId, ServerMessage.Code.FORBIDDEN, "Not a valid method for this request");
					break;
			}
		}

		static void HandleRequest_BlockedUsers(Connection client, ClientMessage.Method method, Guid requestId, string payload)
		{

		}
		static void HandleRequest_FriendRequest(Connection client, ClientMessage.Method method, Guid requestId, string payload)
		{
			// Check if user is logged in
			if (client.Id == Guid.Empty)
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.UNAUTHORIZED, "Not logged in");
				return;
			}

			while (!userAPI.TryLockUser(Guid.Empty))
				Thread.Sleep(10);
			try
			{
				// Get user
				User user = userAPI[client.Id];
				if (user == null)
				{
					SimpleClientResponse(client, requestId, ServerMessage.Code.ERROR, "Server error");
					return;
				}

				// Get/Set username
				switch (method)
				{
					case ClientMessage.Method.GET:
						SimpleClientResponse(client, requestId, ServerMessage.Code.OK, user.Username);
						break;
					case ClientMessage.Method.POST:
						if (string.IsNullOrWhiteSpace(payload))
						{
							SimpleClientResponse(client, requestId, ServerMessage.Code.INVALID_PARAMS, "Username cannot be empty");
							return;
						}
						break;
					default:
						SimpleClientResponse(client, requestId, ServerMessage.Code.FORBIDDEN, "Not a valid method for this request");
						break;
				}
			}
			catch (Exception ex)
			{
				Console.WriteLine($"Exception: {ex.Message}"); // DEBUG
				return;
			}
			finally
			{
				userAPI.UnlockUser(Guid.Empty);
			}

			User thatUser = userAPI[payload];
			if (thatUser != null)
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.OK, "Friendrequest sent");
				return;
			}

			if (thatUser.HasBlocked(client.Id))
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.OK, "Friendrequest sent");
				return;
			}

			if (thatUser.IsFriendsWith(client.Id))
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.NOPE, "Already friends");
				return;
			}

			thatUser.FriendRequests.Add(new DbUser.FriendRequest(client.Id));
			userAPI[thatUser.Id] = thatUser;

			thatUser.SendMessage();
		}
		static void HandleRequest_Friends(Connection client, ClientMessage.Method method, Guid requestId, string payload)
		{
			FriendMessage message;
			try
			{
				message = JsonConvert.DeserializeObject<FriendMessage>(payload);
			}
			catch (Exception)
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.ERROR, "Invalid payload");
				return;
			}

			User thisUser = userAPI[client.Id];
			if (thisUser != null)
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.ERROR, "Server error");
				return;
			}

			if (!thisUser.IsFriendsWith(message.userId))
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.UNAUTHORIZED, "Friend not found");
				return;
			}

			User thatUser = userAPI[client.Id];
			if (thatUser != null)
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.ERROR, "Server error");
				return;
			}

			switch (method)
			{
				case ClientMessage.Method.GET:
					throw new NotImplementedException("MessageGet");
					/*foreach (var msg in thisUser)
					ServerMessage messageObject = new ServerMessage()
					{
						code = code,
						type = ((message == null) ? ServerMessage.DataType.NULL : ServerMessage.DataType.STRING),
						requestId = requestId,
						payload = ((message == null) ? null : Convert.ToBase64String(Encoding.Unicode.GetBytes(message))),
					};

					string jsonMessage = JsonConvert.SerializeObject(messageObject);

					if (client != null && !string.IsNullOrEmpty(jsonMessage))
					{
						client.SendMessage(jsonMessage);
					}*/
					break;
				case ClientMessage.Method.POST:
					message.utcTime = DateTime.UtcNow;
					message.messageId = Guid.NewGuid();
					string jsonMessage = JsonConvert.SerializeObject(message);
					thatUser.SendMessage(jsonMessage);
					break;
				case ClientMessage.Method.DELETE:
					throw new NotImplementedException("MessageDelete");
					break;
				default:
					SimpleClientResponse(client, requestId, ServerMessage.Code.FORBIDDEN, "Not a valid method for this request");
					break;
			}
		}
		static void HandleRequest_FriendsMessage(Connection client, ClientMessage.Method method, Guid requestId, string payload)
		{
			FriendMessage message;
			try
			{
				message = JsonConvert.DeserializeObject<FriendMessage>(payload);
			}
			catch (Exception)
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.ERROR, "Invalid payload");
				return;
			}

			User thisUser = userAPI[client.Id];
			if (thisUser != null)
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.ERROR, "Server error");
				return;
			}

			if (!thisUser.IsFriendsWith(client.Id))
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.UNAUTHORIZED, "Friend not found");
				return;
			}

			User thatUser = userAPI[client.Id];
			if (thatUser != null)
			{
				SimpleClientResponse(client, requestId, ServerMessage.Code.ERROR, "Server error");
				return;
			}

			switch (method)
			{
				case ClientMessage.Method.GET:
					throw new NotImplementedException("MessageGet");
					/*foreach (var msg in thisUser)
					ServerMessage messageObject = new ServerMessage()
					{
						code = code,
						type = ((message == null) ? ServerMessage.DataType.NULL : ServerMessage.DataType.STRING),
						requestId = requestId,
						payload = ((message == null) ? null : Convert.ToBase64String(Encoding.Unicode.GetBytes(message))),
					};

					string jsonMessage = JsonConvert.SerializeObject(messageObject);

					if (client != null && !string.IsNullOrEmpty(jsonMessage))
					{
						client.SendMessage(jsonMessage);
					}*/
					break;
				case ClientMessage.Method.POST:
					message.utcTime = DateTime.UtcNow;
					message.messageId = Guid.NewGuid();
					string jsonMessage = JsonConvert.SerializeObject(message);
					thatUser.SendMessage(jsonMessage);
					break;
				case ClientMessage.Method.DELETE:
					throw new NotImplementedException("MessageDelete");
					break;
				default:
					SimpleClientResponse(client, requestId, ServerMessage.Code.FORBIDDEN, "Not a valid method for this request");
					break;
			}
		}
		static void HandleRequest_P2PConnection(Connection client, ClientMessage.Method method, Guid requestId, string payload)
		{

		}
		static void HandleRequest_FriendSendRPC(Connection client, ClientMessage.Method method, Guid requestId, string payload)
		{

		}
	}
}
