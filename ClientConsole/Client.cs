using System;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace CollarControl
{
	/// <summary>
	/// Client instance
	/// </summary>
	public class Client
	{
		/// <summary>
		/// Returns if the socket is still connected
		/// </summary>
		public Boolean IsConnected
		{
			get
			{
				return _socket?.Connected ?? false;
			}
		}

		private Socket _socket = null;
		private Thread _thread = null;
		private Crypto _crypto = null;
		private ManualResetEvent _connected = new ManualResetEvent(false);
		private ManualResetEvent _receiveDone = new ManualResetEvent(false);

		/// <summary>
		/// Gets invoked on client connect
		/// </summary>
		public event Action<Client> OnConnected;
		/// <summary>
		/// Gets invoked on client disconnect
		/// </summary>
		public event Action<Client> OnDisconnected;
		/// <summary>
		/// Gets invoked when client sends a message
		/// </summary>
		public event Action<String> OnMessageReceived;

		~Client()
		{
			Cleanup();
		}

		/// <summary>
		/// Closes the connection and stops the thread
		/// </summary>
		public void Cleanup()
		{
			if (_socket != null)
			{
				try
				{
					_socket?.Shutdown(SocketShutdown.Both);
					_socket?.Close();
					_socket?.Dispose();
				}
				finally
				{
				}
			}

			if (_thread != null)
			{
				try
				{
					_thread?.Join();
				}
				finally
				{
				}
			}

			_thread = null;
			_socket = null;
			_crypto = null;
		}

		/// <summary>
		/// Connects to the specified uri and port
		/// </summary>
		/// <returns>
		/// Return <see langword="true"/> if connection was successfull
		/// </returns>
		/// <param name="hostUri"></param>
		/// <param name="port"></param>
		/// <exception cref="ArgumentNullException"></exception>
		/// <exception cref="ArgumentOutOfRangeException"></exception>
		/// <exception cref="SocketException"></exception>
		/// <exception cref="System.Security.SecurityException"></exception>
		public Boolean Connect(String hostUri, UInt16 port)
		{
			Cleanup();

			if (hostUri == null)
			{
				throw new ArgumentNullException("HostURI is null.");
			}

			IPAddress[] addresses;

			// Establish the local endpoint for the socket.
			try
			{
				addresses = Dns.GetHostEntry(hostUri).AddressList;
			}
			catch (ArgumentOutOfRangeException)
			{
				throw new ArgumentOutOfRangeException("hostUri is more than 255 characters long.");
			}
			catch (SocketException ex)
			{
				Console.WriteLine("An error was encountered when resolving the hostUri: {0}", ex.Message);
				return false;
			}
			catch (ArgumentException)
			{
				Console.WriteLine("hostUri is an invalid IP address");
				return false;
			}

			if (addresses.Length == 0 || addresses[0] == null)
			{
				return false;
			}

			IPEndPoint remoteEndPoint = new IPEndPoint(addresses[0], port);

			// Create a TCP/IP socket
			_socket = new Socket(addresses[0].AddressFamily, SocketType.Stream, ProtocolType.Tcp);

			// Bind the socket to the local endpoint and listen for incoming connections
			_socket.NoDelay = true;
			_socket.Connect(remoteEndPoint);

			OnConnected.Invoke(this);
			return true;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		public void Authenticate()
		{
			// Create crypto instance
			_crypto = new Crypto();

			// Get public key
			Byte[] clientKey = _crypto.GetPublicKey();

			// Send public key
			try
			{
				SendBytes(clientKey);
			}
			catch (SocketException ex)
			{
				throw new Exception("Could not send public key: " + ex.Message); // @TODO: implement custom exception
			}

			// Receive server public key
			Byte[] serverKey = null;
			try
			{
				serverKey = ReceiveBytes();
			}
			catch (Exception ex)
			{
				throw new Exception("Could not receive remote public key: " + ex.Message); // @TODO: implement custom exception
			}

			if (serverKey == null)
			{
				throw new Exception("Server sent null!"); // @TODO: implement custom exception
			}

			// Generate private key
			try
			{
				_crypto.GenPrivateKey(serverKey);
			}
			catch (Exception ex)
			{
				throw new Exception("Couldn't generate private key: " + ex.Message);
			}
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
				throw new Exception("Could not start receiver handler: " + ex.Message);
			}
		}

		public String ReceiveMessage()
		{
			Byte[] encMessage = null;
			try
			{
				encMessage = ReceiveBytes();
			}
			catch (Exception ex)
			{
				throw new Exception("Couldn't not receive message: " + ex.Message);
			}

			Console.WriteLine("[" + Encoding.UTF8.GetString(encMessage) + "]"); // @DEBUG

			String[] strings = null;

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

			Byte[] messageBytes = null;
			messageBytes = _crypto.Decrypt(Convert.FromBase64String(strings[0]), Convert.FromBase64String(strings[1]));

			if (messageBytes == null)
			{
				throw new Exception("Could not decrypt received data!");
			}

			try
			{
				return Encoding.UTF8.GetString(messageBytes);
			}
			catch (Exception ex)
			{
				throw new Exception("Couldn't not convert message to string: " + ex.Message);
			}
		}

		public void SendMessage(String message)
		{
			Byte[] data = _crypto.Encrypt(Encoding.UTF8.GetBytes(message), out Byte[] iv);

			Byte[] base64 = Encoding.UTF8.GetBytes(Convert.ToBase64String(data) + '\0' + Convert.ToBase64String(iv));

			try
			{
				SendBytes(base64);
			}
			catch (Exception ex)
			{
				throw new Exception("Could not send message: " + ex.Message);
			}
		}

		/// <summary>
		/// Recieves bytes from the client (blocking)
		/// </summary>
		/// <returns>
		/// Returns bytes read
		/// </returns>
		/// <exception cref="SocketException"></exception>
		/// <exception cref="System.Security.SecurityException"></exception>
		/// <exception cref="NotImplementedException"></exception> // @TODO: throw custom exception
		private Byte[] ReceiveBytes()
		{
			Byte[] messageLength = new Byte[2];

			try
			{
				_socket.Receive(messageLength, 0, 2, 0);
			}
			catch (ObjectDisposedException ex)
			{
				throw new NotImplementedException(); // @TODO: Throw custom exception
			}

			UInt16 size = BitConverter.ToUInt16(messageLength, 0);
			Byte[] messageBytes = new Byte[size];

			try
			{
				_socket.Receive(messageBytes, 0, size, 0);
			}
			catch (ObjectDisposedException ex)
			{
				throw new NotImplementedException(); // @TODO: Throw custom exception
			}

			return messageBytes;
		}

		/// <summary>
		/// Sends the bytes to the client
		/// </summary>
		/// <param name="messageBytes"></param>
		/// <exception cref="ArgumentNullException"></exception>
		/// <exception cref="ArgumentOutOfRangeException"></exception>
		/// <exception cref="SocketException"></exception>
		private void SendBytes(Byte[] messageBytes)
		{
			if (messageBytes == null)
			{
				throw new ArgumentNullException();
			}

			if (messageBytes.Length + 2 > UInt16.MaxValue)
			{
				throw new ArgumentOutOfRangeException("message length can not exceed 65535 bytes!");
			}
			if (messageBytes.Length == 0)
			{
				throw new ArgumentOutOfRangeException("message length can not be 0 bytes!");
			}

			// Get message length to use as a header
			Byte[] messageLength = BitConverter.GetBytes((UInt16)messageBytes.Length);

			Byte[] data = new Byte[2 + messageBytes.Length];

			// Combine the arrays
			Array.Copy(messageLength, 0, data, 0, 2);
			Array.Copy(messageBytes, 0, data, 2, messageBytes.Length);

			try
			{
				_socket.Send(data, 0, data.Length, 0);
			}
			catch (ObjectDisposedException ex)
			{
				// @TODO: Throw custom exception
			}
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
					Console.WriteLine("[ReceiveAsync] Could not receive: " + ex.Message);
				}
				_receiveDone.WaitOne();
			}

			OnDisconnected.Invoke(this);
		}

		private void MessageLengthReceivedCallback(IAsyncResult asyncResult)
		{
			try
			{
				StateObject state = (StateObject)asyncResult.AsyncState;
				Int32 bytesRead = 0;
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
					state.bytes = new Byte[size];

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

				Int32 bytesRead = 0;
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
					Byte[] encMessage = state.bytes;

					String[] strings = null;

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

					Byte[] messageBytes = _crypto.Decrypt(
						Convert.FromBase64String(strings[0]),
						Convert.FromBase64String(strings[1])
						);

					if (messageBytes == null)
					{
						throw new Exception("Could not decrypt message");
					}

					_receiveDone.Set();
					Task.Run(() => OnMessageReceived.Invoke(Encoding.UTF8.GetString(messageBytes)));
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
			public Byte[] bytes;

			public StateObject(UInt16 length)
			{
				this.length = length;
				bytes = new Byte[length];
			}
		}
	}
}