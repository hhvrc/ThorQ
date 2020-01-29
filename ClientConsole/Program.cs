using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Threading;

namespace CollarControl
{
	class Program
	{
		static List<Connection> connections = new List<Connection>();

		static void Main(string[] args)
		{
			Console.WriteLine(System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().CodeBase));

			Client client = new Client();

			client.OnConnected += LoginHandler;
			client.OnDisconnected += DisconnectHandler;
			client.OnMessageReceived += MessageReceivedHandler;

			try
			{
#if !DEBUG
				String addr = args[0];
				UInt16 port = UInt16.Parse(args[1]);
#else
				string addr = "localhost";
				ushort port = 5001; ;
#endif

				client.Connect(addr, port);
				Console.WriteLine("Socket connected to {0}:{1}", addr, port);
			}
			catch (Exception ex)
			{
				Console.WriteLine("Couldn't initialize client: " + ex.Message);
			}

			while (client.IsConnected) { Thread.Sleep(500); }

			Console.ReadLine();
		}

		static void MessageReceivedHandler(Client client, string str)
		{
			Console.WriteLine(str);

			Thread.Sleep(5000);

			ClientPayloads.AccountGetRequest ser = new ClientPayloads.AccountGetRequest()
			{
				username = "test",
				password = "test",
			};
			ClientPackage message = new ClientPackage()
			{
				method = RequestMethod.GET,
				request = RequestType.Account,
				payload = ser.Serialize(),
			};

			string msg = JsonConvert.SerializeObject(message);

			if (client != null && !string.IsNullOrEmpty(msg))
			{
				client.SendMessage(msg);
			}
		}

		static void DisconnectHandler(Client client)
		{
			client.Cleanup();
		}

		static void LoginHandler(Client client)
		{
			Console.WriteLine("Logging in...");

			try
			{
				if (client.Authenticate())
				{
					Console.WriteLine("Authenticated!");
				}
				else
				{
					Console.WriteLine("Authentication failed!");
					return;
				}
			}
			catch (Exception ex)
			{
				Console.WriteLine("Could not authenticate: " + ex.ToString());
				return;
			}

			client.StartListening();
		}
	}

	class User
	{
		public string name;

		public User(string name)
		{
			this.name = name;
		}
	}

	class Connection
	{
		public User user;
		public Client connection;

		public Connection(User user, Client connection)
		{
			this.user = user;
			this.connection = connection;
		}
	}
}
