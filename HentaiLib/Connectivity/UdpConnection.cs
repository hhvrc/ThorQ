using System;
using System.Collections.Generic;
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
	public class UdpConnection
	{
		private Socket socket = null;
		private byte[] buffer = new byte[1024];
		private List<EndPoint> clientList = new List<EndPoint>();

		/// <summary>
		/// Gets invoked when a client connects.
		/// You need to start listening for this event to fire.
		/// </summary>
		public event Action<EndPoint, byte[]> OnMessageReceived;
		
		/// <summary>
		/// Starts listening for connecting clients, will invoke <c>OnClientConnected</c> when a client connects. (blocking call)
		/// </summary>
		/// <param name="port"></param>
		/// <exception cref="SocketException"></exception>
		/// <exception cref="SecurityException"></exception>
		/// <exception cref="NotSupportedException"></exception>
		public void Listen(ushort port, bool useIPv6)
		{
			socket = new Socket(
				(useIPv6 ? AddressFamily.InterNetworkV6 : AddressFamily.InterNetwork),
				SocketType.Dgram,
				ProtocolType.Udp
				);

			socket.SetSocketOption(
				SocketOptionLevel.Socket,
				SocketOptionName.ReuseAddress,
				true
				);

			socket.Bind(new IPEndPoint(
				(useIPv6 ? IPAddress.IPv6Any : IPAddress.Any),
				port
				));

			StartListening();
		}

		private void StartListening()
		{
			EndPoint newClientEP = new IPEndPoint(IPAddress.Any, 0);
			socket.BeginReceiveFrom(buffer, 0, buffer.Length, SocketFlags.None, ref newClientEP, HandleMessage, newClientEP);
		}

		private void HandleMessage(IAsyncResult iar)
		{
			try
			{
				EndPoint clientEP = new IPEndPoint(IPAddress.Any, 0);
				int dataLen = socket.EndReceiveFrom(iar, ref clientEP);

				if (!clientList.Exists(client => client.Equals(clientEP)))
					this.clientList.Add(clientEP);

				byte[] data = new byte[dataLen];
				Array.Copy(buffer, data, dataLen);

				try
				{
					Task.Run(() => OnMessageReceived.Invoke(clientEP, data));
				}
				catch (Exception)
				{
				}

				StartListening();
			}
			catch (ObjectDisposedException)
			{
			}
		}

		public void SendTo(byte[] data, EndPoint clientEP)
		{
			try
			{
				socket.SendTo(data, clientEP);
			}
			catch (SocketException)
			{
				this.clientList.Remove(clientEP);
			}
		}

		public void SendToAll(byte[] data)
		{
			foreach (var client in this.clientList)
			{
				this.SendTo(data, client);
			}
		}

		/// <summary>
		/// Stops the listener, and unblocks the thread that called it
		/// </summary>
		public void StopListening()
		{
			socket.Close();
			socket = null;
		}
	}
}
