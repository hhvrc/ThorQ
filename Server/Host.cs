using System;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Security;

namespace CollarControl
{
	/// <summary>
	/// Listen for incoming connections, and create a new <c>Connection</c> object when when a client connects
	/// </summary>
	public class Host
	{
		struct StateObject
		{
			public Socket socket;
			public ManualResetEvent signal;
		}

		/// <summary>
		/// Should it listen to client messages
		/// </summary>
		private volatile bool _listen = false;

		/// <summary>
		/// Gets invoked when a client connects.
		/// You need to start listening for this event to fire.
		/// </summary>
		public event Action<Connection> OnClientConnected;

		~Host()
		{
			_listen = false;
		}

		/// <summary>
		/// Starts listening for connecting clients, will invoke <c>OnClientConnected</c> when a client connects. (blocking call)
		/// </summary>
		/// <param name="port"></param>
		/// <exception cref="SocketException"></exception>
		/// <exception cref="SecurityException"></exception>
		/// <exception cref="NotSupportedException"></exception>
		public void Listen(UInt16 port)
		{
			_listen = true;

			// Establish the local endpoint for the socket.
			IPHostEntry entry = Dns.GetHostEntry(Dns.GetHostName());

			if (entry.AddressList.Length == 0) { return; }

			IPAddress myIP = entry.AddressList[0];

			IPEndPoint localEndPoint = new IPEndPoint(myIP, port);

			// Create a TCP/IP socket.  
			Socket listener = new Socket(myIP.AddressFamily, SocketType.Stream, ProtocolType.Tcp);

			using (ManualResetEvent connected = new ManualResetEvent(false))
			{
				// Bind the socket to the local endpoint and listen for incoming connections.
				listener.Bind(localEndPoint);
				listener.Listen(100);

				while (_listen)
				{
					// Set the event to nonsignaled state
					connected.Reset();
					try
					{
						// Start an asynchronous socket to listen for connections.
						Console.WriteLine("[Server] Waiting for a connection...");
						listener.BeginAccept(new AsyncCallback(ClientInstance), new StateObject { socket = listener, signal = connected });

					}
					catch (Exception ex)
					{
						connected.Set();
						Console.WriteLine("[Server] Could not accept: {0}", ex.Message);
					}
					// Wait until a connection is made before continuing.
					connected.WaitOne();
				}
			}
		}

		/// <summary>
		/// Stops the listener, and unblocks the thread that called it
		/// </summary>
		public void StopListening()
		{
			_listen = false;
		}

		/// <summary>
		/// Gets run when a client connects
		/// </summary>
		/// <param name="ar">
		/// Contains socket of clientconnection
		/// </param>
		private void ClientInstance(IAsyncResult ar)
		{
			// Get the socket that handles the client request
			StateObject state = (StateObject)ar.AsyncState;
			Socket listener = state.socket;

			// Signal the main thread to continue
			if (state.signal == null) { return; }
			state.signal.Set();

			Socket socket;

			if (listener == null) { return; }
			try
			{
				socket = listener.EndAccept(ar);
			}
			catch (Exception ex)
			{
				Console.WriteLine("Couldn't accept client connection: {0}", ex.Message); // DEBUG
				return;
			}

			// Create client object
			Connection client = new Connection(socket);

			// Invoke event
			OnClientConnected?.Invoke(client);
		}
	}
}