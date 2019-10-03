using System;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace CollarControl
{
	public class Host
	{
		private ManualResetEvent _connected = new ManualResetEvent(false);

		public event Action<ClientConnection> OnClientConnected;

		public void StartListening(int port)
		{
			// Establish the local endpoint for the socket.
			IPAddress myIP = Dns.GetHostEntry(Dns.GetHostName()).AddressList[0];
			IPEndPoint localEndPoint = new IPEndPoint(myIP, port);

			// Create a TCP/IP socket.  
			Socket listener = new Socket(myIP.AddressFamily, SocketType.Stream, ProtocolType.Tcp);

			// Bind the socket to the local endpoint and listen for incoming connections.  
			try
			{
				listener.Bind(localEndPoint);
				listener.Listen(100);

				while (true)
				{
					// Set the event to nonsignaled state.  
					_connected.Reset();

					// Start an asynchronous socket to listen for connections.  
					Console.WriteLine("Waiting for a connection...");
					listener.BeginAccept(new AsyncCallback(ClientInstance), listener);

					// Wait until a connection is made before continuing.  
					_connected.WaitOne();
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
			_connected.Set();

			// Get the socket that handles the client request
			Socket listener = (Socket)ar.AsyncState;
			Socket socket = listener.EndAccept(ar);

			// Create client object
			ClientConnection client = new ClientConnection(socket);

			// Invoke event
			OnClientConnected.Invoke(client);
		}
	}

	public class ClientConnection
	{
		/// <summary>
		/// Returns whether the client is still connected
		/// </summary>
		public bool IsConnected
		{
			get
			{
				return _socket?.Connected ?? false;
			}
		}

		private object _idLock = new object();
		private Guid _id;
		/// <summary>
		/// ID attribute
		/// </summary>
		public Guid Id
		{
			get
			{
				lock (_idLock)
					return _id;
			}
			set
			{
				lock (_idLock)
					_id = value;
			}
		}

		/// <summary>
		/// Connection
		/// </summary>
		public event Action<ClientConnection> OnClientDisconnected;

		/// <summary>
		/// Connection, Username, Message
		/// </summary>
		public event Action<ClientConnection, String> OnMessageReceived;

		private Socket _socket = null;
		private Thread _thread = null;
		private Crypto _crypto = null;

		private ManualResetEvent _receiveDone = new ManualResetEvent(false);

		public ClientConnection(Socket socket)
		{
			_socket = socket;
		}

		~ClientConnection()
		{
			Cleanup();
		}

		private void Cleanup()
		{
			try
			{
				_socket?.Shutdown(SocketShutdown.Both);
				_socket?.Close();
				_socket?.Dispose();
			}
			catch (Exception ex)
			{
				Console.WriteLine("Couldnt cleanup socket: " + ex.Message);
			}

			try
			{
				_thread.Join();
			}
			catch (Exception ex)
			{
				Console.WriteLine("Couldnt cleanup thread: " + ex.Message);
			}

			_thread = null;
			_socket = null;
			_crypto = null;
		}

		public bool Authenticate()
		{
			byte[] remoteKey = null;

			try
			{
				remoteKey = ReceiveBytes();
			}
			catch (Exception ex)
			{
				throw new Exception("Could not receive remote public key: " + ex.Message);
			}

			if (remoteKey == null)
			{
				throw new Exception("Client sent null!");
			}

			if (remoteKey.Length <= 0)
			{
				throw new Exception("Client sent empty public key");
			}

			_crypto = new Crypto();
			try
			{
				_crypto.GenPrivateKey(remoteKey);
			}
			catch (Exception ex)
			{
				throw new Exception("Couldn't generate private key: " + ex.Message);
			}

			byte[] pubKey = _crypto.GetPublicKey();

			if (pubKey == null)
			{
				throw new Exception("Could not generate public key");
			}

			try
			{
				SendBytes(pubKey);
			}
			catch (Exception ex)
			{
				throw new Exception("Could not send public key: " + ex.Message);
			}

			return true;
		}

		public void StartListening()
		{
			try
			{
				_thread = new Thread(new ThreadStart(ReceiveAsync));
				_thread.Start();
			}
			catch (Exception ex)
			{
				Console.WriteLine("Could not start receiver handler: " + ex.Message);
			}
		}

		public String ReceiveMessage()
		{
			byte[] encMessage = null;
			try
			{
				encMessage = ReceiveBytes();
			}
			catch (Exception ex)
			{
				Console.WriteLine("Couldn't not receive message: " + ex.Message);
				return null;
			}

			Console.WriteLine("[" + Encoding.UTF8.GetString(encMessage) + "]"); // @DEBUG

			string[] strings = null;

			Console.WriteLine("1");

			try
			{
				strings = Encoding.UTF8.GetString(encMessage).Split('\0');
			}
			catch (Exception ex)
			{
				throw new Exception("Couldn't not convert message to string: " + ex.Message);
			}

			Console.WriteLine("2");

			if (strings.Length != 2)
			{
				throw new Exception("Malformed received data!");
			}

			Console.WriteLine("Str1: " + strings[0]);
			Console.WriteLine("Str2: " + strings[1]);

			byte[] messageBytes = _crypto.Decrypt(
				Convert.FromBase64String(strings[0]),
				Convert.FromBase64String(strings[1])
				);

			if (messageBytes == null)
			{
				Console.WriteLine("Could not decrypt received data!");
				return null;
			}

			try
			{
				return Encoding.UTF8.GetString(messageBytes);
			}
			catch (Exception ex)
			{
				Console.WriteLine("Couldn't not convert message to string: " + ex.Message);
			}
			return null;
		}

		public void SendMessage(string message)
		{
			byte[] data = _crypto.Encrypt(
				Encoding.UTF8.GetBytes(message),
				out byte[] iv);

			byte[] base64 = Encoding.UTF8.GetBytes(Convert.ToBase64String(data) + '\0' + Convert.ToBase64String(iv));

			try
			{
				SendBytes(base64);
			}
			catch (Exception ex)
			{
				throw new Exception("Could not send message: " + ex.Message);
			}
		}

		private byte[] ReceiveBytes()
		{
			byte[] messageLength = new byte[2];

			_socket.Receive(messageLength, 0, messageLength.Length, 0);

			UInt16 size = BitConverter.ToUInt16(messageLength, 0);
			byte[] messageBytes = new byte[size];

			_socket.Receive(messageBytes, 0, size, 0);

			return messageBytes;
		}

		private void SendBytes(byte[] messageBytes)
		{
			// Message should never exceed 64KiB
			if (messageBytes.Length > UInt16.MaxValue) { return; }
			// Convert the string data to byte data using UTF8 encoding.

			byte[] messageLength = BitConverter.GetBytes((UInt16)messageBytes.Length);

			byte[] data = new byte[messageLength.Length + messageBytes.Length];

			Array.Copy(messageLength, 0, data, 0, messageLength.Length);
			Array.Copy(messageBytes, 0, data, messageLength.Length, messageBytes.Length);

			_socket.Send(data, 0, data.Length, 0);
		}

		private void ReceiveAsync()
		{
			while (IsConnected)
			{
				_receiveDone.Reset();
				try
				{
					StateObject state = new StateObject(2);

					_socket.BeginReceive(state.bytes, 0, state.length, 0, new AsyncCallback(MessageLengthReceivedCallback), state);
				}
				catch (Exception ex)
				{
					Console.WriteLine(ex.Message);
				}
				_receiveDone.WaitOne();
			}

			OnClientDisconnected.Invoke(this);
		}

		private void MessageLengthReceivedCallback(IAsyncResult asyncResult)
		{
			try
			{
				StateObject state = (StateObject)asyncResult.AsyncState;
				int bytesRead = 0;
				try
				{
					bytesRead = _socket.EndReceive(asyncResult);
				}
				catch (Exception ex)
				{
					throw new Exception("Could not receive client message: " + ex.Message);
				}

				if (bytesRead == state.length)
				{
					UInt16 size = BitConverter.ToUInt16(state.bytes, 0);

					state.length = size;
					state.bytes = new byte[size];

					try
					{
						_socket.BeginReceive(state.bytes, 0, state.length, 0, new AsyncCallback(MessageReceivedCallback), state);
					}
					catch (Exception ex)
					{
						throw new Exception("Could not receive client message: " + ex.Message);
					}
				}
				else
				{
					_receiveDone.Set();
				}
			}
			catch (Exception ex)
			{
				_receiveDone.Set();
				Console.WriteLine(ex.Message);
			}
		}

		private void MessageReceivedCallback(IAsyncResult asyncResult)
		{
			try
			{
				StateObject state = (StateObject)asyncResult.AsyncState;

				int bytesRead = 0;
				try
				{
					bytesRead = _socket.EndReceive(asyncResult);
				}
				catch (Exception ex)
				{
					throw new Exception("Could not receive client message: " + ex.Message);
				}

				if (bytesRead == state.length)
				{
					byte[] encMessage = state.bytes;

					string[] strings = null;

					try
					{
						strings = Encoding.UTF8.GetString(encMessage).Split('\0');
					}
					catch (Exception ex)
					{
						throw new Exception("Couldn't convert bytes to string: " + ex.Message);
					}

					if (strings.Length != 2)
					{
						throw new Exception("Received message format is invalid");
					}

					byte[] messageBytes = _crypto.Decrypt(
						Convert.FromBase64String(strings[0]),
						Convert.FromBase64String(strings[1])
						);

					if (messageBytes == null)
					{
						throw new Exception("Could not decrypt message");
					}

					_receiveDone.Set();
					OnMessageReceived.Invoke(this, Encoding.UTF8.GetString(messageBytes));
				}
			}
			catch (Exception ex)
			{
				_receiveDone.Set();
				Console.WriteLine(ex.Message);
			}
		}

		private class StateObject
		{
			public UInt16 length;
			public byte[] bytes;

			public StateObject(UInt16 length)
			{
				this.length = length;
				bytes = new byte[length];
			}
		}
	}
}