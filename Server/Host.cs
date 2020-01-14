using System;
using System.Net;
using System.Net.Sockets;
using System.Security;
using System.Threading;

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

		private ManualResetEvent _connected = new ManualResetEvent(false);

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
		public void Listen(ushort port)
		{
			_listen = true;
			
			if (port >= UInt16.MaxValue - 1)
			{
				Console.WriteLine("Port too high!");
				return;
			}
			
			IPEndPoint ipv4Endpoint = new IPEndPoint(IPAddress.Any, port);
			Socket ipv4Listener = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
			ipv4Listener.Bind(ipv4Endpoint);
			ipv4Listener.Listen(16);
			/*
			IPEndPoint ipv6Endpoint = new IPEndPoint(IPAddress.IPv6Any, port);
			Socket ipv6Listener = new Socket(AddressFamily.InterNetworkV6, SocketType.Stream, ProtocolType.Tcp);
			ipv6Listener.Bind(ipv6Endpoint);
			ipv6Listener.Listen(16);
			*/
			while (_listen)
			{
				// Set the event to nonsignaled state
				_connected.Reset();

				try
				{
				// Start an asynchronous socket to listen for connections.

				ipv4Listener.BeginAccept(new AsyncCallback(ClientInstance), new StateObject { socket = ipv4Listener });
				//ipv6Listener.BeginAccept(new AsyncCallback(ClientInstance), new StateObject { socket = ipv6Listener });
				}
				catch (Exception ex)
				{
					_connected.Set();
					Console.WriteLine("[Server] Could not accept: {0}", ex.Message);
				}
				// Wait until a connection is made before continuing.
				_connected.WaitOne();
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

			Socket socket;

			if (listener == null)
			{
				_connected?.Set();
				return;
			}

			try
			{
				socket = listener.EndAccept(ar);
			}
			catch (Exception ex)
			{
				Console.WriteLine("Couldn't accept client connection: {0}", ex.Message); // DEBUG
				_connected?.Set();
				return;
			}
			_connected?.Set();

			// Create client object
			Connection client = new Connection(socket);

			// Invoke event
			OnClientConnected?.Invoke(client);
		}
	}
}
 