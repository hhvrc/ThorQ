using CollarLib;
using Newtonsoft.Json;
using HeavenLib;
using HeavenLib.Connectivity;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CollarControl
{
	class Program
	{
		class Instance
		{
			public Instance(HostConnection connection, String displayname)
			{
				this.connection = connection;
				this.displayname = displayname;
			}
			public String instanceId { get; } = ToolBox.CompressGuid(Guid.NewGuid());
			public String displayname { get; set; }
			public HostConnection connection { get; set; }
			public String incomingRequest { get; set; } = null;
		}

		static Host host = null;
		static Random random = new Random();

		static List<Instance> instances = new List<Instance>();
		static List<HostConnection> newConnections = new List<HostConnection>();

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
		
		// Client event handlers
		public static void OnClientConnected(HostConnection client)
		{
			Console.WriteLine("\n[Client] Connected");

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

			lock (newConnections)
				newConnections.Add(client);

			client.OnClientDisconnected += OnClientDisconnected;
			client.OnMessageReceived += OnClientMessageReceived;

			client.StartListening();
		}
		public static void OnClientDisconnected(HostConnection client)
		{
			Console.WriteLine("[Client] Disconnected");
			client.OnClientDisconnected -= OnClientDisconnected;
			client.OnMessageReceived -= OnClientMessageReceived;
			lock (newConnections)
				newConnections.RemoveAll(c => c == client);
			lock (instances)
				instances.RemoveAll(i => i.connection == client);
			client.Dispose();
		}
		public static void SimpleClientResponse(HostConnection client, ResponseType type, string payload)
		{
			var messageObject = new Response()
			{
				type = type,
				payload = payload,
			};

			client.SendEncrypted(Encoding.UTF8.GetBytes(messageObject.Serialize()));
		}
		public static void OnClientMessageReceived(HostConnection client, byte[] str)
		{
			try
			{
				var msg = Request.Deserialize(Encoding.UTF8.GetString(str));

				switch (msg.type)
				{
					case RequestType.Ping:
						SimpleClientResponse(client, ResponseType.Ping, "");
						break;
					case RequestType.Register:
						lock (newConnections)
							newConnections.RemoveAll(c => c == client);
						lock (instances)
						{
							var instance = instances.FirstOrDefault(i => i.connection == client);
							if (instance == null)
							{
								instance = new Instance(client, msg.Payload);
								instances.Add(instance);
							}
							SimpleClientResponse(client, ResponseType.Key, instance.instanceId);
						}
						break;
					case RequestType.Request:
						lock (instances)
						{
							var sender = instances.FirstOrDefault(i => i.connection == client);
							var receiver = instances.FirstOrDefault(i => i.instanceId == msg.Payload);
							if (receiver == null)
							{
								SimpleClientResponse(client, ResponseType.Error, "Invalid requestkey!");
								break;
							}
							if (receiver.connection == sender.connection)
							{
								SimpleClientResponse(client, ResponseType.Error, "Cannot request on self!");
								break;
							}

							receiver.incomingRequest = msg.Payload;

							SimpleClientResponse(client, ResponseType.Ok, "");
							SimpleClientResponse(receiver.connection, ResponseType.Request, sender.displayname);
						}
						break;
					case RequestType.Accept:
						lock (instances)
						{
							var accepter = instances.FirstOrDefault(i => i.connection == client);
							if (accepter.incomingRequest == null)
							{
								SimpleClientResponse(client, ResponseType.Error, "No request received yet!");
								break;
							}

							var requester = instances.FirstOrDefault(i => i.instanceId == accepter.incomingRequest);
							if (requester == null)
							{
								SimpleClientResponse(client, ResponseType.Error, "Requesting person went offline!");
								break;
							}

							accepter.incomingRequest = null;
							SimpleClientResponse(accepter.connection,  ResponseType.P2PInfo, requester.connection.address());
							SimpleClientResponse(requester.connection, ResponseType.P2PInfo,  accepter.connection.address());
						}
						break;
					case RequestType.Deny:
						lock (instances)
						{
							var denyer = instances.FirstOrDefault(i => i.connection == client);
							if (denyer.incomingRequest == null)
							{
								SimpleClientResponse(client, ResponseType.Error, "No request received yet!");
								break;
							}

							var requester = instances.FirstOrDefault(i => i.instanceId == denyer.incomingRequest);
							if (requester == null)
							{
								SimpleClientResponse(client, ResponseType.Error, "Requesting person went offline!");
								break;
							}

							denyer.incomingRequest = null;
							SimpleClientResponse(client, ResponseType.Ok, "");
							SimpleClientResponse(requester.connection, ResponseType.Denied, "");
						}
						break;
					default:
						SimpleClientResponse(client, ResponseType.Error, "Invalid request!");
						break;
				}
			}
			catch (Exception ex)
			{
				SimpleClientResponse(client, ResponseType.Error, "Oops, something happened!");
				Console.WriteLine($"[Client] Could not receive message: {ex.Message}");
			}
		}
	}
}
