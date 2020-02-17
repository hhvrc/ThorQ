using Newtonsoft.Json;
using HeavenLib.Connectivity;
using System;
using System.Collections.Concurrent;

namespace CollarControl
{
	public static class Connection
	{
		static object cliLock = new object();
		static Client client = null;
		static ConcurrentDictionary<Guid, Action<CollarLib.Response>> responseCallbacks = new ConcurrentDictionary<Guid, Action<CollarLib.Response>>();
		static void MessageHandler(Client client, string payload)
		{
			var resp = JsonConvert.DeserializeObject<CollarLib.Response>(payload);

			if (responseCallbacks.TryGetValue(resp.requestId, out var action))
			{
				action(resp);
				return;
			}

			ServerMessageReceived?.Invoke(resp);
		}
		static void DisconnectHandler(Client client)
		{
			Disconnected?.Invoke();

			if (client != null)
			{
				try { client?.Cleanup(); } catch (Exception) { }
				client = null;
			}
		}

		public static event Action Disconnected;
		public static event Action<CollarLib.Response> ServerMessageReceived;

		public static bool IsConnected
		{
			get
			{
				lock (cliLock)
					return client?.IsConnected ?? false;
			}
		}
		public static string ServerHostname
		{
			get
			{
				if (AppCache.TryGet("ServerHostname", out string str))
					return str;
				return null;
			}
			set
			{
				AppCache.Upsert("ServerHostname", ((value == null) ? "" : value));
			}
		}
		public static ushort ServerPort
		{
			get
			{
				if (AppCache.TryGet("ServerPort", out string str))
					if (ushort.TryParse(str, out ushort port))
						return port;
				return 0;
			}
			set
			{
				AppCache.Upsert("ServerPort", value.ToString());
			}
		}

		public static bool Connect()
		{
			lock (cliLock)
			{
				if (client != null)
				{
					try { client?.Cleanup(); } catch (Exception) { }
					client = null;
				}
				try
				{
					client = new Client();
					if (client.Connect(ServerHostname, ServerPort) && client.Authenticate())
					{
						client.OnMessageReceived += MessageHandler;
						client.OnDisconnected += DisconnectHandler;
						client.StartListening();
						return true;
					}
				}
				catch (Exception) { }
				try { client?.Cleanup(); } catch (Exception) { }
				client = null;
				return false;
			}
		}
		public static void Disconnect()
		{
			lock (cliLock)
			{
				try { client?.Cleanup(); } catch (Exception) { }
				client = null;
			}
		}
		public static void SendMessage(String payload, CollarLib.RequestMethod requestMethod, CollarLib.RequestType requestType, Action<CollarLib.Response> onResponse)
		{
			CollarLib.ClientRequest req = new CollarLib.ClientRequest
			{
				Id = Guid.NewGuid(),
				Method = requestMethod,
				Request = requestType,
				Payload = payload
			};

			responseCallbacks.TryAdd(req.Id, onResponse);

			String message = req.Serialize();

			lock (cliLock)
			{
				if (client != null)
				{
					client.SendMessage(message);
				}
			}
		}
		public static bool TestAddress(string hostname, ushort port)
		{
			try
			{
				Client client = new Client();
				if (client.Connect(hostname, port) && client.Authenticate())
					return true;
			}
			catch (Exception) { }
			finally
			{
				client?.Cleanup();
			}
			return false;
		}
	}
}
