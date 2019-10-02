using System;
using System.IO.Ports;
using System.Text;
using System.Threading;
using System.Collections.Generic;

namespace CollarControl
{
    class Program
	{
		static List<Connection> connections = new List<Connection>();

		static void Main(string[] args)
        {
            Console.WriteLine(System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().CodeBase));

			Host<User> host = new Host<User>();
			host.OnClientDisconnected += DisconnectHandler;
			host.OnClientConnected += LoginHandler;

			host.StartListening(10235);




            //CredentialHandler handler = new CredentialHandler();

            //handler.SetCredentials("HeavenVR", "user@example.com", "password");
            //CredentialHandler.Creds creds = handler.GetCredentials();
            //handler.SetCredentials("HeavenVR", "user@example.com", "password");
            //creds = handler.GetCredentials();

            //Console.WriteLine(creds.username);
            //Console.WriteLine(creds.passwordHash);
            //Console.WriteLine(creds.mailAddress);
        }

		static void DisconnectHandler(object caller, Host<User>.ClientConnection client)
		{
			foreach (Connection conn in connections)
			{
				try
				{
					conn.connection?.SendMessage(String.Format("User \"{0}\" has disconnected!", client.Payload.name));
				}
				catch (Exception ex)
                {
                    Console.WriteLine("[" + conn.user.name + "] " + ex.Message);
                    conn.connection = null;
                }
			}
		}

		static void LoginHandler(object caller, Host<User>.ClientConnection client)
		{
            client.Authenticate();

			String username = client.ReceiveMessage();

			if (String.IsNullOrWhiteSpace(username))
			{
				return;
			}

			User user = new User(username);

			connections.Add(new Connection(user, client));

            client.StartListening(user);

			foreach	(Connection conn in connections)
			{
                try
                {
                    conn.connection?.SendMessage(String.Format("User \"{0}\" has logged on!", username));
                }
                catch(Exception ex)
                {
                    Console.WriteLine("[" + conn.user.name + "] " + ex.Message);
                    conn.connection = null;
                }
			}
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
		public Host<User>.ClientConnection connection;

		public Connection(User user, Host<User>.ClientConnection connection)
		{
			this.user = user;
			this.connection = connection;
		}
	}
}
