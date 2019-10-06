using System;
using System.Collections.Generic;
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
		public event Action<Connection> OnClientDisconnected;

		/// <summary>
		/// Connection, Username, Message
		/// </summary>
		public event Action<Connection, String> OnMessageReceived;

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
			// Create crypto instance
			_crypto = new Crypto();

			// Receive client public key
			byte[] clientKey = null;
			try
			{
				clientKey = ReceiveBytes();
			}
			catch (Exception ex)
			{
				throw new Exception("Could not receive remote public key: " + ex.Message);
			}
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
			try
			{
				SendBytes(serverKey);
			}
			catch (Exception ex)
			{
				throw new Exception("Could not send public key: " + ex.Message);
			}

			// Generate private key
			try
			{
				_crypto.GenPrivateKey(clientKey);
			}
			catch (Exception ex)
			{
				throw new Exception("Couldn't generate private key: " + ex.Message);
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
				throw new Exception("Couldn't not receive message: " + ex.Message);
			}

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

			byte[] messageBytes = _crypto.Decrypt(Convert.FromBase64String(strings[0]), Convert.FromBase64String(strings[1]));

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

		public void SendMessage(string message)
		{
			byte[] data = _crypto.Encrypt(Encoding.UTF8.GetBytes(message), out byte[] iv);

			if (data == null)
			{
				return;
			}

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

			OnClientDisconnected?.Invoke(this); // NOTE Crashes if null
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

					byte[] messageBytes = _crypto.Decrypt(Convert.FromBase64String(strings[0]), Convert.FromBase64String(strings[1]));

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
