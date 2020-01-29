using System;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace CollarControl
{
	/// <summary>
	/// Connection instance
	/// </summary>
	public class Connection : IDisposable
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

		private Socket _socket = null;
		private Thread _thread = null;
		private Crypto _crypto = null;

		/// <summary>
		/// Gets invoked on connection established
		/// </summary>
		public event Action<Connection> OnClientDisconnected;

		/// <summary>
		/// Gets invoked on connection lost
		/// </summary>
		public event Action<Connection, string> OnMessageReceived;


		private ManualResetEvent _receiveDone = new ManualResetEvent(false);

		public Connection(Socket socket)
		{
			_socket = socket;
		}

		/// <summary>
		/// Closes the connection and stops the thread
		/// </summary>
		public void Dispose()
		{
			// We dont care if anything fails... just shut it down
			try { _socket?.Shutdown(SocketShutdown.Both); } catch (Exception) { }
			try { _socket?.Close(); } catch (Exception) { }
			try { _socket?.Dispose(); } catch (Exception) { }
			try { _thread?.Join(); } catch (Exception) { }
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
				_crypto.EstablishSecretKey(clientKey);
			}
			catch (Exception ex)
			{
				throw new Exception("Couldn't generate private key: " + ex.Message);
			}

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

		public string ReceiveMessage()
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

			byte[] messageBytes = _crypto.Decrypt(encMessage);

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
			byte[] data = _crypto.Encrypt(Encoding.UTF8.GetBytes(message));

			if (data == null)
			{
				return;
			}

			try
			{
				SendBytes(data);
			}
			catch (Exception ex)
			{
				throw new Exception("Could not send message: " + ex.Message);
			}
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
				catch (Exception ex)
				{
					Console.WriteLine("Client error: {0}", ex.Message);
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
			catch (Exception ex)
			{
				_receiveDone.Set();
				Console.WriteLine("Client error: {0}", ex.Message);
			}
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
			catch (Exception ex)
			{
				_receiveDone.Set();
				Console.WriteLine("Client error: {0}", ex.Message);
			}
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
