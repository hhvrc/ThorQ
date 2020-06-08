using HeavenLib.Connectivity;
using System;
using System.Text;

namespace ThorQ
{
	public static class Connection
	{
		static object cliLock = new object();
		static Client client = null;
		static void MessageHandler(Client client, byte[] payload)
		{
			MessageReceived?.Invoke(Encoding.UTF8.GetString(payload));
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
		public static event Action<string> MessageReceived;

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
				AppCache.Upsert("ServerPort", ((value == null) ? "" : value));
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
				if (client != null)
				{
					try { client?.Cleanup(); } catch (Exception) { }
					client = null;
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
