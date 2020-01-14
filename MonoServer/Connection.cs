using System;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace CollarControl
{
	public class Connection
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
		private Guid _id = Guid.Empty;
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
		public event Action<Connection> OnClientDisconnected;

		/// <summary>
		/// Connection, Username, Message
		/// </summary>
		public event Action<Connection, string> OnMessageReceived;

		private Socket _socket = null;
		private Thread _thread = null;
		private Crypto _crypto = null;

		private ManualResetEvent _receiveDone = new ManualResetEvent(false);

		public Connection(Socket socket)
		{
			_socket = socket;
		}

		~Connection()
		{
			Cleanup();
		}

		private void Cleanup()
		{
#if !DEBUG
			try
			{
#endif
				_socket?.Shutdown(SocketShutdown.Both);
#if !DEBUG
			}
			catch (Exception) { }
#endif

#if !DEBUG
			try
			{
#endif
				_socket?.Close();
#if !DEBUG
			}
			catch (Exception) { }
#endif

#if !DEBUG
			try
			{
#endif
				_socket?.Dispose();
#if !DEBUG
			}
			catch (Exception) { }
#endif

#if !DEBUG
			try
			{
#endif
				_thread.Join();
#if !DEBUG
			}
			catch (Exception) { }
#endif

			_thread = null;
			_socket = null;
			_crypto = null;
		}

		public bool Authenticate()
		{
			// Create crypto instance
			_crypto = new Crypto();

			// Receive client public key
			byte[] clientKey = null;
#if !DEBUG
			try
			{
#endif
				clientKey = ReceiveBytes();
#if !DEBUG
			}
			catch (Exception ex)
			{
				throw new Exception("Could not receive remote public key: " + ex.Message);
			}
#endif
			if (clientKey == null)
			{
				throw new Exception("Client sent null!");
			}
			if (clientKey.Length <= 0)
			{
				throw new Exception("Client sent empty public key");
			}

			// Get public key
			byte[] serverKey = _crypto.GetPublicKey();
			if (serverKey == null)
			{
				throw new Exception("Could not generate public key");
			}

			// Send public key
#if !DEBUG
			try
			{
#endif
				SendBytes(serverKey);
#if !DEBUG
			}
			catch (Exception ex)
			{
				throw new Exception("Could not send public key: " + ex.Message);
			}
#endif

			// Generate private key
#if !DEBUG
			try
			{
#endif
				_crypto.EstablishSecretKey(clientKey);
#if !DEBUG
			}
			catch (Exception ex)
			{
				throw new Exception("Couldn't generate private key: " + ex.Message);
			}
#endif

			string message = ReceiveMessage(); // TODO Fix: Possible freezing of application

			if (string.IsNullOrEmpty(message))
			{
				message = "Error";
			}
			SendMessage(message);

			Console.WriteLine("[AUTH] Received: {0}", message);

			return message == "ACK";
		}

		public void StartListening()
		{
#if !DEBUG
			try
			{
#endif
				_thread = new Thread(new ThreadStart(ReceiveAsync));
				_thread.Start();
#if !DEBUG
			}
			catch (Exception ex)
			{
				Console.WriteLine("Could not start receiver handler: " + ex.Message);
			}
#endif
		}

		public void StopListening()
		{
			Cleanup();
		}

		public string ReceiveMessage()
		{
			byte[] encMessage = null;
#if !DEBUG
			try
			{
#endif
				encMessage = ReceiveBytes();
#if !DEBUG
			}
			catch (Exception ex)
			{
				throw new Exception("Couldn't not receive message: " + ex.Message);
			}
#endif

			byte[] messageBytes = _crypto.Decrypt(encMessage);

			if (messageBytes == null)
			{
				throw new Exception("Could not decrypt received data!");
			}

#if !DEBUG
			try
			{
#endif
				return Encoding.UTF8.GetString(messageBytes);
#if !DEBUG
			}
			catch (Exception ex)
			{
				throw new Exception("Couldn't not convert message to string: " + ex.Message);
			}
#endif
		}

		public void SendMessage(string message)
		{
			byte[] data = _crypto.Encrypt(Encoding.UTF8.GetBytes(message));

			if (data == null)
			{
				return;
			}

#if !DEBUG
			try
			{
#endif
				SendBytes(data);
#if !DEBUG
			}
			catch (Exception ex)
			{
				throw new Exception("Could not send message: " + ex.Message);
			}
#endif
		}

		private byte[] ReceiveBytes()
		{
			byte[] messageLength = new byte[2];

			_socket.Receive(messageLength, 0, 2, 0);

			ushort size = BitConverter.ToUInt16(messageLength, 0);
			byte[] messageBytes = new byte[size];

			_socket.Receive(messageBytes, 0, size, 0);

			return messageBytes;
		}

		private void SendBytes(byte[] messageBytes)
		{
			// Message should never exceed 64KiB
			if (messageBytes.Length > ushort.MaxValue) { return; }
			// Convert the string data to byte data using UTF8 encoding.

			byte[] messageLength = BitConverter.GetBytes((ushort)messageBytes.Length);

			byte[] data = new byte[2 + messageBytes.Length];

			Array.Copy(messageLength, 0, data, 0, 2);
			Array.Copy(messageBytes, 0, data, 2, messageBytes.Length);

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
				catch (SocketException)
				{
					Console.WriteLine("Connection lost!");
				}
#if !DEBUG
				catch (Exception ex)
				{
				Console.WriteLine("Client error: {0}", ex.Message);
				}
#endif
				_receiveDone.WaitOne();
			}

			OnClientDisconnected?.Invoke(this); // NOTE Crashes if null
		}

		private void MessageLengthReceivedCallback(IAsyncResult asyncResult)
		{
			try
			{
				StateObject state = (StateObject)asyncResult.AsyncState;
				int bytesRead = 0;

				bytesRead = _socket.EndReceive(asyncResult);

				if (bytesRead == state.length)
				{
					ushort size = BitConverter.ToUInt16(state.bytes, 0);

					state.length = size;
					state.bytes = new byte[size];

					_socket.BeginReceive(state.bytes, 0, state.length, 0, new AsyncCallback(MessageReceivedCallback), state);
				}
				else
				{
					_receiveDone.Set();
				}
			}
			catch (SocketException)
			{
				_receiveDone.Set();
				Console.WriteLine("Connection lost!");
			}
#if !DEBUG
			catch (Exception ex)
			{
			_receiveDone.Set();
				Console.WriteLine("Client error: {0}", ex.Message);
			}
#endif
		}

		private void MessageReceivedCallback(IAsyncResult asyncResult)
		{
			try
			{
				StateObject state = (StateObject)asyncResult.AsyncState;

				int bytesRead = 0;

					bytesRead = _socket.EndReceive(asyncResult);

				if (bytesRead == state.length)
				{
					byte[] messageBytes = _crypto.Decrypt(state.bytes);

					if (messageBytes == null)
					{
						throw new Exception("Could not decrypt message");
					}

					_receiveDone.Set();
					OnMessageReceived.Invoke(this, Encoding.UTF8.GetString(messageBytes));
				}
			}
			catch (SocketException)
			{
				_receiveDone.Set();
				Console.WriteLine("Connection lost!");
			}
#if !DEBUG
			catch (Exception ex)
			{
			_receiveDone.Set();
				Console.WriteLine("Client error: {0}", ex.Message);
			}
#endif
		}

		private class StateObject
		{
			public ushort length;
			public byte[] bytes;

			public StateObject(ushort length)
			{
				this.length = length;
				bytes = new byte[length];
			}
		}
	}
}
