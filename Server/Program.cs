using CollarLib;
using Newtonsoft.Json;
using HeavenLib;
using HeavenLib.Connectivity;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Threading.Tasks;

namespace ThorQ
{
	class Program
	{
		public static Host host = null;
		public static UserAPI userAPI = null;
		public static ConversationAPI conversationAPI = null;
		public static Random random = new Random();

		public static ConcurrentDictionary<Guid, RuntimeUser> initializedUsers;
		public static List<HostConnection> nonAuthedConnections = new List<HostConnection>();

		public static EmailClient emailClient;

		static void Main(string[] args)
		{
			if (!AppConfig.TryGet("smtp_host", out var smtpHost)         || String.IsNullOrWhiteSpace(smtpHost)    ||
				!AppConfig.TryGet("smtp_port", out var smtpPortStr)      || String.IsNullOrWhiteSpace(smtpPortStr) ||
				!AppConfig.TryGet("smtp_email", out var smtpEmail)       || String.IsNullOrWhiteSpace(smtpEmail)   ||
				!AppConfig.TryGet("smtp_password", out var smtpPassword) || String.IsNullOrWhiteSpace(smtpPassword))
			{
				AppConfig.EnsureKey("smtp_host");
				AppConfig.EnsureKey("smtp_port");
				AppConfig.EnsureKey("smtp_email");
				AppConfig.EnsureKey("smtp_password");
				Console.WriteLine("please fill out smtp info in \"config.json\"");
				Console.ReadLine();
				return;
			}
			if (!ToolBox.IsValidEmail(smtpEmail))
			{
				Console.WriteLine("email in \"config.json\" is invalid");
				Console.ReadLine();
				return;
			}
			if (!int.TryParse(smtpPortStr, out int smtpPort))
			{
				Console.WriteLine("port in \"config.json\" is invalid");
				Console.ReadLine();
				return;
			}
			emailClient = new EmailClient(smtpHost, smtpPort, smtpEmail, smtpPassword);

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

			var db = new LiteDB.LiteDatabase(Path.Combine(thisPath, "MyData.db"));

			userAPI = new UserAPI(db);
			conversationAPI = new ConversationAPI(db);

			userAPI.GetAllUsers((IEnumerable<DbUser> users) =>
			{
				foreach (var usr in users)
				{
					var user = new RuntimeUser(usr.Id);
					user.IsOnlineChanged += OnUserOnlineChanged;
					user.MessageReceived += OnUserMessageReceived;
					initializedUsers.TryAdd(usr.Id, user);
				}
			});

			initializedUsers = new ConcurrentDictionary<Guid, RuntimeUser>();

			host.OnClientConnected += OnClientConnected;

			try
			{
				host.Listen(port, useIPv6);
			}
			catch (Exception ex)
			{
				Console.WriteLine($"Server crashed: {ex.Message}");
			}
		}

		public static void SimpleClientResponse(HostConnection client, Guid requestId, ResponseCode code, string message)
		{
			var messageObject = new Response()
			{
				code = code,
				type = ((message == null) ? ResponseType.NULL : ResponseType.STRING),
				requestId = requestId,
				payload = message,
			};

			client.SendEncrypted(Encoding.UTF8.GetBytes(messageObject.Serialize()));
		}

		// Client event handlers
		public static void OnClientConnected(HostConnection client)
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
					client?.Dispose();
					return;
				}
			}
			catch (Exception ex)
			{
				Console.WriteLine($"[Client] Could not authenticate: {ex.Message}");
				client?.Dispose();
				return;
			}

			lock (nonAuthedConnections)
				nonAuthedConnections.Add(client);

			client.OnClientDisconnected += OnClientDisconnected;
			client.OnMessageReceived += OnClientMessageReceived;

			client.StartListening();
		}
		public static void OnClientDisconnected(HostConnection client)
		{
			client.OnClientDisconnected -= OnClientDisconnected;
			client.OnMessageReceived -= OnClientMessageReceived;
			nonAuthedConnections.Remove(client);
			client.Dispose();
		}
		public static void OnClientMessageReceived(HostConnection client, byte[] str)
		{
			try
			{
				var msg = ClientRequest.Deserialize(Encoding.UTF8.GetString(str));

				switch (msg.Request)
				{
					case RequestType.Account:
						Account_RequestHandler.Dispatch(null, client, msg.Method, msg.Id, msg.Payload);
						break;
					case RequestType.Recovery:
						Recovery_RequestHandler.Dispatch(null, client, msg.Method, msg.Id, msg.Payload);
						break;
					case RequestType.Username:
					case RequestType.Email:
					case RequestType.Password:
					case RequestType.BlockedUsers:
					case RequestType.FriendRequest:
					case RequestType.Friends:
					case RequestType.Conversation:
					case RequestType.Message:
					case RequestType.P2PRequest:
					case RequestType.RPC:
						SimpleClientResponse(client, msg.Id, ResponseCode.UNAUTHORIZED, "Not logged in");
						break;
					default:
						SimpleClientResponse(client, msg.Id, ResponseCode.INVALID_REQUEST, "Not a valid request");
						break;
				}
			}
			catch (Exception ex)
			{
				SimpleClientResponse(client, Guid.Empty, ResponseCode.NOPE, "Invalid message");
				Console.WriteLine($"[Client] Could not receive message: {ex.Message}");
			}
		}

		// User event handlers
		public static void OnUserMessageReceived(RuntimeUser thisUser, HostConnection client, string str)
		{
			try
			{
				var msg = ClientRequest.Deserialize(str);

				switch (msg.Request)
				{
					case RequestType.Account:
						Account_RequestHandler.Dispatch(thisUser, client, msg.Method, msg.Id, msg.Payload);
						break;
					case RequestType.Recovery:
						Recovery_RequestHandler.Dispatch(thisUser, client, msg.Method, msg.Id, msg.Payload);
						break;
					case RequestType.Username:
						Username_RequestHandler.Dispatch(thisUser, client, msg.Method, msg.Id, msg.Payload);
						break;
					case RequestType.Email:
						Email_RequestHandler.Dispatch(thisUser, client, msg.Method, msg.Id, msg.Payload);
						break;
					case RequestType.Password:
						Password_RequestHandler.Dispatch(thisUser, client, msg.Method, msg.Id, msg.Payload);
						break;
					case RequestType.BlockedUsers:
						BlockedUser_RequestHandler.Dispatch(thisUser, client, msg.Method, msg.Id, msg.Payload);
						break;
					case RequestType.FriendRequest:
						FriendRequest_RequestHandler.Dispatch(thisUser, client, msg.Method, msg.Id, msg.Payload);
						break;
					case RequestType.Friends:
						Friend_RequestHandler.Dispatch(thisUser, client, msg.Method, msg.Id, msg.Payload);
						break;
					case RequestType.Conversation:
						Conversation_RequestHandler.Dispatch(thisUser, client, msg.Method, msg.Id, msg.Payload);
						break;
					case RequestType.Message:
						Message_RequestHandler.Dispatch(thisUser, client, msg.Method, msg.Id, msg.Payload);
						break;
					case RequestType.P2PRequest:
						P2PRequest_RequestHandler.Dispatch(thisUser, client, msg.Method, msg.Id, msg.Payload);
						break;
					case RequestType.RPC:
						RPC_RequestHandler.Dispatch(thisUser, client, msg.Method, msg.Id, msg.Payload);
						break;
					default:
						SimpleClientResponse(client, msg.Id, ResponseCode.INVALID_REQUEST, "Not a valid request");
						break;
				}
			}
			catch (Exception ex)
			{
				SimpleClientResponse(client, Guid.Empty, ResponseCode.ERROR, "Invalid payload");
				Console.WriteLine($"[Client] Could not receive message: {ex.Message}");
			}
		}
		public static void OnUserOnlineChanged(RuntimeUser user, bool online)
		{
			userAPI.GetById(user.Id,
				(dbUser)=>
				{
					var friendMsg = new CollarLib.ServerPayloads.Friend()
					{
						UserId = dbUser.Id,
						Username = dbUser.Username,
						Activity = (online ? dbUser.Activity : UserActivity.Offline),
						Status = dbUser.Status
					};
					var message = new Response()
					{
						code = ResponseCode.UPDATE_DATA,
						type = ResponseType.FRIEND,
						requestId = Guid.Empty,
						payload = friendMsg.Serialize(),
					};
					var jsonMessage = message.Serialize();

					foreach (Guid id in dbUser.Friends)
						if (initializedUsers.TryGetValue(id, out RuntimeUser friend))
							friend.SendMessage(jsonMessage);
				},
				(err)=>
				{
					Console.WriteLine($"OnUserOnlineChanged(): {err}");
				});

		}

		public void BroadcastNotification(string subject, string body)
		{
			try
			{
				var message = new Response()
				{
					code = ResponseCode.ADMIN_MSG,
					type = ResponseType.STRING,
					requestId = Guid.Empty,
					payload = $"{Convert.ToBase64String(Encoding.Unicode.GetBytes(subject))}_{Convert.ToBase64String(Encoding.Unicode.GetBytes(body))}"
				};

				var jsonMessage = JsonConvert.SerializeObject(message);

				var users = initializedUsers.Values;

				foreach (var user in users)
				{
					try
					{
						user.SendMessage(jsonMessage);
					}
					catch (Exception ex)
					{
						userAPI.GetById(user.Id,
						(dbUser)=>
						{
							Console.WriteLine($"Error broadcasting notification to { dbUser.Username }: {ex.Message}");
						},
						(err)=>{});
					}
				}
			}
			catch (Exception ex)
			{
				Console.WriteLine($"Error starting notification broadcast: {ex.Message}");
			}
		}

	}
}
