using System;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace CollarControl
{
	/// <summary>
	/// Listen for incoming connections, and create a new <c>Connection</c> object when when a client connects
	/// </summary>
	public class Host
	{
		private ManualResetEvent _connected = new ManualResetEvent(false);

		/// <summary>
		/// Gets invoked when a client connects.
		/// You need to start listening for this event to fire.
		/// </summary>
		public event Action<Connection> OnClientConnected;

		/// <summary>
		/// Starts listening for connecting clients, will invoke <c>OnClientConnected</c> when a client connects.
		/// </summary>
		/// <param name="port"></param>
		public void StartListening(int port)
		{
			// Establish the local endpoint for the socket.
			IPAddress myIP = Dns.GetHostEntry(Dns.GetHostName()).AddressList[0]; // UNHANDLED
			IPEndPoint localEndPoint = new IPEndPoint(myIP, port); // UNHANDLED

			// Create a TCP/IP socket.  
			Socket listener = new Socket(myIP.AddressFamily, SocketType.Stream, ProtocolType.Tcp); // UNHANDLED

			// Bind the socket to the local endpoint and listen for incoming connections.  
			try
			{
				listener.Bind(localEndPoint); // UNHANDLED
				listener.Listen(100); // UNHANDLED

				while (true)
				{
					// Set the event to nonsignaled state.  
					_connected.Reset(); // UNHANDLED

					// Start an asynchronous socket to listen for connections.  
					Console.WriteLine("Waiting for a connection..."); // UNHANDLED
					listener.BeginAccept(new AsyncCallback(ClientInstance), listener); // UNHANDLED

					// Wait until a connection is made before continuing.  
					_connected.WaitOne(); // UNHANDLED
				}

			}
			catch (Exception ex)
			{
				Console.WriteLine(ex.Message);
			}
		}

		public void ClientInstance(IAsyncResult ar)
		{
			// Signal the main thread to continue
			_connected.Set(); // UNHANDLED

			// Get the socket that handles the client request
			Socket listener = (Socket)ar.AsyncState;
			Socket socket = listener.EndAccept(ar); // UNHANDLED

			// Create client object
			Connection client = new Connection(socket);

			// Invoke event
			OnClientConnected.Invoke(client);
		}
	}
}