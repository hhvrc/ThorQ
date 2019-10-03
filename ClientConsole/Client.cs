using System;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace CollarControl
{
	public class Client
	{
		public bool IsConnected
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

		public event Action<Client> OnConnected; // @TODO change to action
		public event Action<Client> OnClientDisconnected;
		public event Action<String> OnMessageReceived;

		~Client()
		{
			Cleanup();
		}

		public void Cleanup()
		{
			if (_socket != null)
			{
				try
				{
					_socket.Shutdown(SocketShutdown.Both);
					_socket.Close();
					_socket.Dispose();
				}
				catch (Exception ex)
				{
					Console.WriteLine("Couldnt cleanup socket: " + ex.Message);
				}
			}

			if (_thread != null)
			{
				try
				{
					_thread.Join();
				}
				catch (Exception ex)
				{
					Console.WriteLine("Couldnt cleanup thread: " + ex.Message);
				}
			}

			_thread = null;
			_socket = null;
			_crypto = null;
		}

		public void Connect(String hostUri, int port)
		{
			Cleanup();

			IPAddress ipAddress = null;
			IPEndPoint remoteEndPoint = null;

			// Establish the local endpoint for the socket.
			try
			{
				ipAddress = Dns.GetHostEntry(hostUri).AddressList[0];
				remoteEndPoint = new IPEndPoint(ipAddress, port);
			}
			catch (Exception ex)
			{
				throw new Exception("Could not find host: " + ex.Message);
			}

			// Create a TCP/IP socket
			try
			{
				_socket = new Socket(ipAddress.AddressFamily, SocketType.Stream, ProtocolType.Tcp);
			}
			catch (Exception ex)
			{
				throw new Exception("Couldn't make socket" + ex.Message);
			}

			// Bind the socket to the local endpoint and listen for incoming connections.  
			try
			{
				_socket.NoDelay = true;
				_socket.Connect(remoteEndPoint);
			}
			catch (Exception ex)
			{
				throw new Exception("Couldn't connect to server: " + ex.Message);
			}
			Console.WriteLine("Socket connected to {0}:{1}", hostUri, port);
			OnConnected.Invoke(this);
		}

		public void Authenticate()
		{
			_crypto = new Crypto();

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
				throw new Exception("Server sent null!");
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
			byte[] encMessage = null;
			try
			{
				encMessage = ReceiveBytes();
			}
			catch (Exception ex)
			{
				throw new Exception("Couldn't not receive message: " + ex.Message);
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

			byte[] messageBytes = null;
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

		public void SendMessage(string message)
		{
			byte[] data = _crypto.Encrypt(Encoding.UTF8.GetBytes(message), out byte[] iv);

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
					Console.WriteLine("[ReceiveAsync] Could not receive: " + ex.Message);
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
			public byte[] bytes;

			public StateObject(UInt16 length)
			{
				this.length = length;
				bytes = new byte[length];
			}
		}
	}
}