using System;
using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;
using static CollarControl.ToolBox;

namespace CollarControl
{
	class Program
	{
		public static Host host = null;
		public static UserAPI userAPI = null;
		public static ConversationAPI conversationAPI = null;
		public static Random random = new Random();
		public static List<Connection> nonAuthedConnections = new List<Connection>();

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
			LiteDB.LiteDatabase db = new LiteDB.LiteDatabase(Path.Combine(thisPath, "MyData.db"), null);
			userAPI = new UserAPI(db, OnUserOnlineChanged, OnUserMessageReceived);
			conversationAPI = new ConversationAPI(db);

			host.OnClientConnected += (Connection con) =>
				{
					Task.Run(() => OnClientConnected(con));
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

		public static void SimpleClientResponse(Connection client, Guid requestId, ResponseCode code, string message)
		{
			ServerPackage messageObject = new ServerPackage()
			{
				code = code,
				type = ((message == null) ? ResponseDataType.NULL : ResponseDataType.STRING),
				requestId = requestId,
				payload = message,
			};

			client.SendMessage(messageObject.Serialize());
		}

		// Client event handlers
		public static void OnClientConnected(Connection client)
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
		public static void OnClientDisconnected(Connection client)
		{
			client.OnClientDisconnected -= OnClientDisconnected;
			client.OnMessageReceived -= OnClientMessageReceived;
			nonAuthedConnections.Remove(client);
		}
		public static void OnClientMessageReceived(Connection client, string str)
		{
			try
			{
				ClientPackage msg = ClientPackage.Deserialize(str);

				switch (msg.request)
				{
					case RequestType.Account:
						Account_RequestHandler.Dispatch(null, client, msg.method, msg.id, msg.payload);
						break;
					case RequestType.Recovery:
						Recovery_RequestHandler.Dispatch(null, client, msg.method, msg.id, msg.payload);
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
						SimpleClientResponse(client, msg.id, ResponseCode.UNAUTHORIZED, "Not logged in");
						break;
					default:
						SimpleClientResponse(client, msg.id, ResponseCode.INVALID_REQUEST, "Not a valid request");
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
		public static void OnUserMessageReceived(RuntimeUser thisUser, Connection client, string str)
		{
			try
			{
				ClientPackage msg = ClientPackage.Deserialize(str);

				switch (msg.request)
				{
					case RequestType.Account:
						Account_RequestHandler.Dispatch(thisUser, client, msg.method, msg.id, msg.payload);
						break;
					case RequestType.Recovery:
						Recovery_RequestHandler.Dispatch(thisUser, client, msg.method, msg.id, msg.payload);
						break;
					case RequestType.Username:
						Username_RequestHandler.Dispatch(thisUser, client, msg.method, msg.id, msg.payload);
						break;
					case RequestType.Email:
						Email_RequestHandler.Dispatch(thisUser, client, msg.method, msg.id, msg.payload);
						break;
					case RequestType.Password:
						Password_RequestHandler.Dispatch(thisUser, client, msg.method, msg.id, msg.payload);
						break;
					case RequestType.BlockedUsers:
						BlockedUser_RequestHandler.Dispatch(thisUser, client, msg.method, msg.id, msg.payload);
						break;
					case RequestType.FriendRequest:
						FriendRequest_RequestHandler.Dispatch(thisUser, client, msg.method, msg.id, msg.payload);
						break;
					case RequestType.Friends:
						Friend_RequestHandler.Dispatch(thisUser, client, msg.method, msg.id, msg.payload);
						break;
					case RequestType.Conversation:
						Conversation_RequestHandler.Dispatch(thisUser, client, msg.method, msg.id, msg.payload);
						break;
					case RequestType.Message:
						Message_RequestHandler.Dispatch(thisUser, client, msg.method, msg.id, msg.payload);
						break;
					case RequestType.P2PRequest:
						P2PRequest_RequestHandler.Dispatch(thisUser, client, msg.method, msg.id, msg.payload);
						break;
					case RequestType.RPC:
						RPC_RequestHandler.Dispatch(thisUser, client, msg.method, msg.id, msg.payload);
						break;
					default:
						SimpleClientResponse(client, msg.id, ResponseCode.INVALID_REQUEST, "Not a valid request");
						break;
				}
			}
			catch (Exception ex)
			{
				SimpleClientResponse(client, Guid.Empty, ResponseCode.ERROR, "Invalid payload");
				Console.WriteLine($"[Client] Could not receive message: {ex.Message}");
			}
		}
		public static void OnUserOnlineChanged(RuntimeUser thisUser, bool online)
		{
			ServerPayloads.Friend friendMsg = new ServerPayloads.Friend()
			{
				userId = thisUser.Id,
				username = thisUser.Username,
				state = (online ? thisUser.state : UserActivity.Offline),
				status = thisUser.status
			};
			ServerPackage message = new ServerPackage()
			{
				code = ResponseCode.UPDATE_DATA,
				type = ResponseDataType.FRIEND,
				requestId = Guid.Empty,
				payload = friendMsg.Serialize(),
			};
			string jsonMessage = message.Serialize();

			foreach (Guid id in thisUser.Friends)
			{
				RuntimeUser friend;
				friend = userAPI.GetById(id);

				if (friend != null)
					friend.SendMessage(jsonMessage);
			}
		}
	}
}
