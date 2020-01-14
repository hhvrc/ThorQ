using System;
using System.IO.Ports;
using System.Text;
using System.Threading;
using System.Collections.Generic;
using Newtonsoft.Json;

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

#if !DEBUG
            try
            {
#endif
			String addr = args[0];
			UInt16 port = UInt16.Parse(args[1]);

				client.Connect(addr, port);
				Console.WriteLine("Socket connected to {0}:{1}", addr, port);
#if !DEBUG
			}
			catch (Exception ex)
            {
                Console.WriteLine("Couldn't initialize client: " + ex.Message);
            }
#endif
			

			while (client.IsConnected) { Thread.Sleep(500); }

			Console.ReadLine();
        }

		static void MessageReceivedHandler(Client client, String str)
		{
			Console.WriteLine(str);

			Thread.Sleep(5000);

			Message message = new Message()
			{
				Command = "logout",
				Parameters = new Dictionary<String, String>()
				{
					{ "logoutall", "false" }
				}
			};

			String msg = JsonConvert.SerializeObject(message);

			if (client != null && !String.IsNullOrEmpty(msg))
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

#if !DEBUG
            try
            {
#endif
                if (client.Authenticate())
				{
					Console.WriteLine("Authenticated!");
				}
				else
				{
					Console.WriteLine("Authentication failed!");
					return;
				}

			Message message = new Message()
			{
				Command = "login",
				Parameters = new Dictionary<String, String>()
				{
					{ "username", "test" },
					{ "password", "Passw" }
				}
			};
			/*Message message = new Message()
			{
				Command = "register",
				Parameters = new Dictionary<String, String>()
				{
					{ "email", "user@example.com" },
					{ "username", "test" },
					{ "password", "Passw" }
				}
			};*/

			String msg = JsonConvert.SerializeObject(message);

			if (client != null && !String.IsNullOrEmpty(msg))
			{
				client.SendMessage(msg);
			}
#if !DEBUG
            }
            catch (Exception ex)
            {
                Console.WriteLine("Could not authenticate: " + ex.ToString());
            }
#endif

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
