using System;
using System.Net;
using System.Net.Sockets;
using System.Security;
using System.Threading;
using System.Threading.Tasks;

namespace HeavenLib.Connectivity
{
	/// <summary>
	/// Listen for incoming connections, and create a new <c>Connection</c> object when when a client connects
	/// </summary>
	public class Host
	{
		/// <summary>
		/// Should it listen to client messages
		/// </summary>
		private volatile bool _listen = false;

		private ManualResetEvent _connected = new ManualResetEvent(false);

		/// <summary>
		/// Gets invoked when a client connects.
		/// You need to start listening for this event to fire.
		/// </summary>
		public event Action<HostConnection> OnClientConnected;

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
		public void Listen(ushort port, bool useIPv6)
		{
			_listen = true;

			IPEndPoint endPoint = new IPEndPoint(
				(useIPv6 ? IPAddress.IPv6Any : IPAddress.Any),
				port
				);

			Socket listener = new Socket(
				(useIPv6 ? AddressFamily.InterNetworkV6 : AddressFamily.InterNetwork),
				SocketType.Stream,
				ProtocolType.Tcp
				);

			listener.Bind(endPoint);
			listener.Listen(32);

			while (_listen)
			{
				// Set the event to nonsignaled state
				_connected.Reset();

				try
				{
					// Start an asynchronous socket to listen for connections.

					IAsyncResult s = listener.BeginAccept(new AsyncCallback(ClientInstance), listener);
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
			Socket listener = (Socket)ar.AsyncState;

			Socket socket;
			
			try
			{
				if (listener == null)
					return;

				socket = listener.EndAccept(ar);
			}
			catch (Exception ex)
			{
				Console.WriteLine("Couldn't accept client connection: {0}", ex.Message); // DEBUG
				return;
			}
			finally
			{
				_connected?.Set();
			}

			// Create client object
			HostConnection client = new HostConnection(socket);

			// Invoke event
			Task.Run(() => OnClientConnected?.Invoke(client));
		}
	}
}
