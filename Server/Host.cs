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
			IPAddress myIP = Dns.GetHostEntry(Dns.GetHostName()).AddressList[0]; // @UNHANDLED_ERROR (2)
			IPEndPoint localEndPoint = new IPEndPoint(myIP, port); // @UNHANDLED_ERROR

			// Create a TCP/IP socket.  
			Socket listener = new Socket(myIP.AddressFamily, SocketType.Stream, ProtocolType.Tcp); // @UNHANDLED_ERROR

			// Bind the socket to the local endpoint and listen for incoming connections.  
			try
			{
				listener.Bind(localEndPoint); // @UNHANDLED_ERROR
				listener.Listen(100); // @UNHANDLED_ERROR

				while (true)
				{
					// Set the event to nonsignaled state.  
					_connected.Reset(); // @UNHANDLED_ERROR

					// Start an asynchronous socket to listen for connections.  
					Console.WriteLine("Waiting for a connection..."); // @UNHANDLED_ERROR
					listener.BeginAccept(new AsyncCallback(ClientInstance), listener); // @UNHANDLED_ERROR

					// Wait until a connection is made before continuing.  
					_connected.WaitOne(); // @UNHANDLED_ERROR
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
			_connected.Set(); // @UNHANDLED_ERROR

			// Get the socket that handles the client request
			Socket listener = (Socket)ar.AsyncState;
			Socket socket = listener.EndAccept(ar); // @UNHANDLED_ERROR

			// Create client object
			Connection client = new Connection(socket);

			// Invoke event
			OnClientConnected.Invoke(client);
		}
	}
}