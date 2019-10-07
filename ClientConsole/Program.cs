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


			client.OnMessageReceived += (Client cli, String str) =>
			{
				Message msg = new Message { Command = "Ping" };
				msg.Parameters = new Dictionary<string, string>();
				cli.SendMessage(JsonConvert.SerializeObject(msg));
			};
            client.OnDisconnected += DisconnectHandler;
            client.OnConnected += LoginHandler;

            try
            {
                client.Connect("127.0.0.1", 5001);
				Console.WriteLine("Socket connected to {0}:{1}", "192.168.1.43", 10235);
			}
			catch (Exception ex)
            {
                Console.WriteLine("Couldn't initialize client: " + ex.Message);
            }
			

			while (client.IsConnected) { Thread.Sleep(500); }

			/*CredentialHandler handler = new CredentialHandler();

            handler.SetCredentials("HeavenVR", "user@example.com", "password");
            CredentialHandler.Creds creds = handler.GetCredentials();
            handler.SetCredentials("HeavenVR", "user@example.com", "password");
            creds = handler.GetCredentials();

            Console.WriteLine(creds.username);
            Console.WriteLine(creds.passwordHash);
            Console.WriteLine(creds.mailAddress);*/

			Console.ReadLine();
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
				}
            }
            catch (Exception ex)
            {
                Console.WriteLine("Could not authenticate: " + ex.ToString());
            }

			/*Console.Write("command: ");
			String cmd = Console.ReadLine();

			Console.Write("email: ");
			String email = Console.ReadLine();

			Console.Write("Username: ");
			String username = Console.ReadLine();

			Console.Write("password: ");
			String password = Console.ReadLine();

			Message msg = new Message()
			{
				Command = cmd,
				Parameters = new Dictionary<String, String>()
				{
					{ "email", email },
					{ "username", username },
					{ "password", password }
				}
			};

			String message = JsonConvert.SerializeObject(msg);

			Console.WriteLine(message.ToString());*/
			Message msg = new Message { Command = "Ping" };
			msg.Parameters = new Dictionary<string, string>();
			client.SendMessage(JsonConvert.SerializeObject(msg));

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
