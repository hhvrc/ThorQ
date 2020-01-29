using CollarControl.ServerPayloads;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Windows.Forms;

namespace CollarControl
{
	public class Instance
	{
		public Guid id;
		public string name;
		public string status;
		public UserActivity activity;
		public List<Friend> friends = new List<Friend>();
		public List<BlockedUser> blockedUsers = new List<BlockedUser>();
		public List<FriendRequest> friendRequests = new List<FriendRequest>();
		public List<Conversation> conversations = new List<Conversation>();
		public IPAddress activeP2PConnection = null;
	}

	public static class Connection
	{
		static object cliLock = new object();
		static Client client = null;
		static void MessageHandler(Client client, string payload)
		{
			MessageReceived?.Invoke(payload);
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

	public static class AppState
	{
		private static List<WindowType> appStack = new List<WindowType>();

		public enum WindowType
		{
			None,
			Login,
			Main,
			Options,
			Recovery,
			Register,
		}

		public static void QueueWindow(WindowType type)
		{
			lock (appStack)
				appStack.Add(type);
		}
		public static void CloseWindow(WindowType type)
		{
			lock (appStack)
				if (appStack.Count > 0 && appStack[appStack.Count-1] == type)
					appStack.RemoveAt(appStack.Count-1);
		}
		public static void ExitApplication()
		{
			lock (appStack)
				appStack.Clear();
		}
		public static bool IsExiting()
		{
			lock (appStack)
				return appStack.Count == 0;
		}
		public static WindowType PendingWindow()
		{
			lock (appStack)
				if (appStack.Count > 0)
					return appStack[appStack.Count-1];
			return WindowType.None;
		}

		public static Instance instance = null;

		public static string serverHostname = "";

		public static void Cleanup()
		{

		}
	}

	public static class AppCache
	{
		static private Dictionary<string, string> appCache = File.Exists("cache.txt") ?
				   JsonConvert.DeserializeObject<Dictionary<string, string>>(
					   File.ReadAllText("cache.txt", System.Text.Encoding.UTF8))
				   :
				   new Dictionary<string, string>();
		public static void Upsert(string key, string value)
		{
			lock (appCache)
			{
				bool updated = false;
				if (appCache.TryGetValue(key, out string oldvalue))
				{
					if (oldvalue != value)
					{
						updated = true;
						appCache[key] = value;
					}
				}
				else
				{
					updated = true;
					appCache.Add(key, value);
				}
				if (updated)
					File.WriteAllText("cache.txt", JsonConvert.SerializeObject(appCache, Formatting.Indented));
			}
		}
		public static bool TryGet(string key, out string value)
		{
			lock (appCache)
				return appCache.TryGetValue(key, out value);
		}
		public static string GetOrDefault(string key)
		{
			if (TryGet(key, out string value))
				return value;
			return "";
		}
	}

	static class Program
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main()
		{
			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);

			AppState.QueueWindow(AppState.WindowType.Login);

			Form activeForm = null;
			while (!AppState.IsExiting())
			{
				switch (AppState.PendingWindow())
				{
					case AppState.WindowType.Login:
						activeForm = new LoginForm();
						break;
					case AppState.WindowType.Main:
						activeForm = new MainForm();
						break;
					case AppState.WindowType.Options:
						activeForm = new OptionsForm();
						break;
					case AppState.WindowType.Recovery:
						activeForm = new RecoveryForm();
						break;
					case AppState.WindowType.Register:
						activeForm = new RegisterForm();
						break;
				}
				Application.Run(activeForm);
				activeForm.Dispose();
			}

			AppState.Cleanup();
			Connection.Disconnect();
		}
	}
}
