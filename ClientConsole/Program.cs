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

            Client client = new Client();

            //client.OnMessageReceived += ???
            client.OnClientDisconnected += DisconnectHandler;
            client.OnConnected += LoginHandler;

            try
            {
                client.Connect("127.0.0.1", 10235);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Couldn't initialize client: " + ex.Message);
            }



            //CredentialHandler handler = new CredentialHandler();

            //handler.SetCredentials("HeavenVR", "user@example.com", "password");
            //CredentialHandler.Creds creds = handler.GetCredentials();
            //handler.SetCredentials("HeavenVR", "user@example.com", "password");
            //creds = handler.GetCredentials();

            //Console.WriteLine(creds.username);
            //Console.WriteLine(creds.passwordHash);
            //Console.WriteLine(creds.mailAddress);
        }

        static void DisconnectHandler(Client client)
        {

        }

        static void LoginHandler(Client client)
        {
            try
            {
                client.Authenticate();
            }
            catch (Exception ex)
            {
                Console.WriteLine("Could not log in: " + ex.Message);
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
