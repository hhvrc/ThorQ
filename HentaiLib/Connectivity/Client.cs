using System;
using System.Net;
using System.Net.Sockets;
using System.Security;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using HeavenLib.Security;

namespace HeavenLib.Connectivity
{
	/// <summary>
	/// Client instance
	/// </summary>
	public class Client
	{
		/// <summary>
		/// Returns if the socket is still connected
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
		public event Action<Client, byte[]> OnMessageReceived;

		/// <summary>
		/// Closes the connection and stops the thread
		/// </summary>
		public void Cleanup()
		{
			// We dont care if anything fails... just shut it down
			try { _socket?.Shutdown(SocketShutdown.Both); } catch (Exception) { }
			try { _socket?.Close(); } catch (Exception) { }
			try { _socket?.Dispose(); } catch (Exception) { }
			try { _thread?.Abort(); } catch (Exception) { }
			try { _crypto?.Dispose(); } catch (Exception) { }
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
		public bool Connect(string hostUri, ushort port)
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
			_socket.Connect(remoteEndPoint);

			OnConnected?.Invoke(this);
			return true;
		}

		/// <summary>
		/// Perfroms ECDH key-exhange
		/// </summary>
		/// <returns>
		/// Returns true if authentication succeeded
		/// </returns>
		public bool Authenticate()
		{
			// Create crypto instance
			_crypto = new Crypto();

			// Get public key
			byte[] clientKey = _crypto.GetPublicKey();

			// Send public key
			try
			{
				SendRaw(clientKey);
			}
			catch (SocketException ex)
			{
				Console.WriteLine("SocketException Caught!");
				Console.WriteLine("Could not send public key: " + ex.Message);
				_crypto = null;
				return false;
			}

			// Receive server public key
			byte[] serverKey = null;
			try
			{
				serverKey = ReceiveRaw();
			}
			catch (SecurityException ex)
			{
				Console.WriteLine("SecurityException Caught!");
				Console.WriteLine("Could not send public key: " + ex.Message);
				_crypto = null;
				return false;
			}
			catch (SocketException ex)
			{
				Console.WriteLine("SocketException Caught!");
				Console.WriteLine("Could not send public key: " + ex.Message);
				_crypto = null;
				return false;
			}

			if (serverKey == null)
			{
				Console.WriteLine("Server sent null!");
				_crypto = null;
				return false;
			}

			// Generate private key
			try
			{
				_crypto.EstablishSecretKey(serverKey);
			}
			catch (Exception ex)
			{
				Console.WriteLine("Couldn't generate private key: " + ex.Message);
				_crypto = null;
				return false;
			}

			SendEncrypted(Encoding.UTF8.GetBytes("ACK"));

			return Encoding.UTF8.GetString(ReceiveEncrypted()) == "ACK";
		}

		/// <summary>
		/// Starts message listener thread
		/// </summary>
		/// <exception cref="ThreadStateException"></exception>
		/// <exception cref="OutOfMemoryException"></exception>
		public void StartListening()
		{
			_thread = new Thread(new ThreadStart(ReceiveAsync));
			_thread.Start();
		}

		/// <summary>
		/// Reads a incoming message from client (blocking call)
		/// </summary>
		/// <returns>
		/// Returns the read message, or null if it could not be decrypted
		/// </returns>
		public byte[] ReceiveEncrypted()
		{
			if (_crypto == null) // DEBUG
			{
				Console.WriteLine("Cannot receive message, please authenticate first"); // DEBUG
				return null; // DEBUG
			}

			byte[] encMessage = null;
			try
			{
				encMessage = ReceiveRaw();
			}
			catch (Exception ex)
			{
				return null; // TODO should maybe do something else? (catch when ReceiveBytes() is finished)
			}

			byte[] messageBytes = null;
			try
			{
				messageBytes = _crypto.Decrypt(encMessage);
			}
			catch (Exception ex)
			{
				Console.WriteLine("ReceiveMessage -> splitting: " + ex.Message); // DEBUG
				return null;
			}

			return messageBytes;
		}

		/// <summary>
		/// Sends message to client
		/// </summary>
		/// <param name="message">
		/// Message to send to client
		/// </param>
		public void SendEncrypted(byte[] message)
		{
			if (_crypto == null) // DEBUG
			{
				Console.WriteLine("Cannot send message, please authenticate first"); // DEBUG
				return; // DEBUG
			}

			byte[] data;
			try
			{
				data = _crypto?.Encrypt(message);
			}
			catch (EncoderFallbackException ex)
			{
				Console.WriteLine("Could not get bytes: " + ex.Message); // DEBUG
				Console.WriteLine(ex.HelpLink); // DEBUG
				return;
			}

			if (data == null)
			{
				Console.WriteLine("Encryption failed!"); // DEBUG
				return;
			}

			try
			{
				SendRaw(data);
			}
			catch (SocketException ex)
			{
				Console.WriteLine(ex.HelpLink); // DEBUG
				Console.WriteLine("Could not send message: " + ex.Message); // DEBUG
			}
		}

		/// <summary>
		/// Recieves bytes from the client (blocking call)
		/// </summary>
		/// <returns>
		/// Returns bytes read, or null if it failed
		/// </returns>
		/// <exception cref="SocketException"></exception>
		/// <exception cref="SecurityException"></exception>
		private byte[] ReceiveRaw()
		{
			byte[] data = new byte[2];

			try
			{
				_socket.Receive(data, 0, 2, 0);
			}
			catch (ObjectDisposedException)
			{
				return null;
			}

			ushort size = BitConverter.ToUInt16(data, 0);
			data = new byte[size];

			try
			{
				_socket.Receive(data, 0, size, 0);
			}
			catch (ObjectDisposedException)
			{
				return null;
			}

			return data;
		}

		/// <summary>
		/// Sends bytes to the client
		/// </summary>
		/// <param name="messageBytes"></param>
		/// <exception cref="ArgumentNullException"></exception>
		/// <exception cref="ArgumentOutOfRangeException"></exception>
		/// <exception cref="SocketException"></exception>
		private void SendRaw(byte[] messageBytes)
		{
			if (messageBytes == null)
			{
				throw new ArgumentNullException();
			}

			if (messageBytes.Length + 2 > ushort.MaxValue)
			{
				throw new ArgumentOutOfRangeException("message length can not exceed 65535 bytes!");
			}

			if (messageBytes.Length == 0)
			{
				return;
			}

			// Get message length to use as a header
			byte[] messageLength = BitConverter.GetBytes((ushort)messageBytes.Length);

			byte[] data = new byte[2 + messageBytes.Length];

			// Combine the arrays
			Array.Copy(messageLength, 0, data, 0, 2);
			Array.Copy(messageBytes, 0, data, 2, messageBytes.Length);

			try
			{
				_socket.BeginSend(data, 0, data.Length, 0, new AsyncCallback(SendBytesCallback), null);
			}
			catch (ObjectDisposedException)
			{
				return;
			}
		}

		private void SendBytesCallback(IAsyncResult asyncResult)
		{
			_socket.EndSend(asyncResult);
		}

		/// <summary>
		/// Listens to incoming messages, and starts async receivers (blocking call)
		/// </summary>
		private void ReceiveAsync()
		{
			using (ManualResetEvent receiveDone = new ManualResetEvent(false))
			{
				while (IsConnected)
				{
					receiveDone.Reset();
					StateObject state = new StateObject();
					state.length = 2;
					state.bytes = new byte[2];
					state.signal = receiveDone;

					try
					{
						_socket.BeginReceive(state.bytes, 0, state.length, 0, new AsyncCallback(MessageLengthReceivedCallback), state);
					}
					catch (Exception ex)
					{
						Console.WriteLine("Exception caught: {0}", ex.Message);
					}
					receiveDone.WaitOne();
				}
			}
			OnDisconnected.Invoke(this);
		}

		private void MessageLengthReceivedCallback(IAsyncResult asyncResult)
		{
			StateObject state = (StateObject)asyncResult.AsyncState;
			int bytesRead = 0;

			try
			{
				bytesRead = _socket.EndReceive(asyncResult);
			}
			catch (Exception ex)
			{
				Console.WriteLine("Could not receive client message: " + ex.Message);
				try { state.signal.Set(); } catch (Exception) { }
				return;
			}

			if (bytesRead == state.length)
			{
				state.length = BitConverter.ToUInt16(state.bytes, 0);

				state.bytes = new byte[state.length];

				try
				{
					_socket.BeginReceive(state.bytes, 0, state.length, 0, new AsyncCallback(MessageReceivedCallback), state);
					return;
				}
				catch (Exception ex)
				{
					Console.WriteLine("Could not receive client message: " + ex.Message);
				}
			}
			try { state.signal.Set(); } catch (Exception) { }
		}

		private void MessageReceivedCallback(IAsyncResult asyncResult)
		{
			StateObject state = (StateObject)asyncResult.AsyncState;
			int bytesRead = 0;

			try
			{
				bytesRead = _socket.EndReceive(asyncResult);
			}
			catch (Exception ex)
			{
				Console.WriteLine("Could not receive client message: " + ex.Message);
				try { state.signal.Set(); } catch (Exception) { }
				return;
			}

			if (bytesRead == state.length)
			{
				byte[] encMessage = state.bytes;
				byte[] messageBytes = null;

				try
				{
					messageBytes = _crypto.Decrypt(encMessage);
				}
				catch (Exception ex)
				{
					Console.WriteLine("Received message format is invalid: " + ex.Message);
					try { state.signal.Set(); } catch (Exception) { }
					return;
				}

				try { state.signal.Set(); } catch (Exception) { }
				Task.Run(() => OnMessageReceived.Invoke(this, messageBytes));
				return;
			}
			else
			{
				Console.WriteLine("Message length mismatch!");
			}
			try { state.signal.Set(); } catch (Exception) { }
		}

		private class StateObject
		{
			public ushort length;
			public byte[] bytes;
			public ManualResetEvent signal;
		}
	}
}