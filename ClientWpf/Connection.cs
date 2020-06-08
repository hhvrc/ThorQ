using Newtonsoft.Json;
using HeavenLib.Connectivity;
using System;
using System.Collections.Concurrent;
using System.Text;

namespace ThorQ
{
	public static class Connection
	{
		static object cliLock = new object();
		static Client client = null;
		static ConcurrentDictionary<Guid, Action<CollarLib.Response>> responseCallbacks = new ConcurrentDictionary<Guid, Action<CollarLib.Response>>();
		static void MessageHandler(Client client, byte[] payload)
		{
			var resp = JsonConvert.DeserializeObject<CollarLib.Response>(Encoding.UTF8.GetString(payload));

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
		public static string Hostname
		{
			get
			{
				if (HeavenLib.AppConfig.TryGet("ServerHostname", out string str))
					return str;
				return null;
			}
			set
			{
				HeavenLib.AppConfig.Upsert("ServerHostname", ((value == null) ? "" : value));
			}
		}
		public static ushort Port
		{
			get
			{
				if (HeavenLib.AppConfig.TryGet("ServerPort", out string str))
					if (ushort.TryParse(str, out ushort port))
						return port;
				return 0;
			}
			set
			{
				HeavenLib.AppConfig.Upsert("ServerPort", value.ToString());
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
					if (client.Connect(Hostname, Port) && client.Authenticate())
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
				client?.SendEncrypted(Encoding.UTF8.GetBytes(message));
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
