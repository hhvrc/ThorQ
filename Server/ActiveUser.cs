using System;
using System.Collections.Generic;
using System.Text;

namespace CollarControl
{
	class ActiveUser
	{
		public Guid id;
		public String name;
		private List<Connection> _connections;
		public event Action<Guid, String> OnMessageReceived;

		private void HandleMessage(Connection connection, String message)
		{
			connection.SendMessage(message + "Ack.");
		}

		public ActiveUser(string name, Connection connection)
		{
			this.name = name;
			this.id = connection.Id;
			_connections = new List<Connection>();
			_connections.Add(connection);

			connection.OnMessageReceived += HandleMessage;
		}

		public void AddConnection(Connection connection)
		{
			lock (_connections)
			{
				if (!_connections.Contains(connection))
				{
					_connections.Add(connection);
					connection.OnMessageReceived += HandleMessage;
				}
			}
		}

		public void RemoveConnection(Connection connection)
		{
			lock (_connections)
			{
				connection.OnMessageReceived -= HandleMessage;
				_connections.Remove(connection);
			}
		}

		public void SendMessage(String message)
		{
			lock (_connections)
			{
				foreach (Connection connection in _connections)
				{
					connection.SendMessage(message);
				}
			}
		}

		public void DisconnectHandler(Connection connection)
		{
			bool removeUser = false;
			lock (_connections)
			{
				_connections.Remove(connection);
				if (_connections.Count == 0)
				{
					removeUser = true;
				}
			}
			if (removeUser)
			{
				Program._activeUsers.TryRemove(this.id, out _);
			}
		}
	}
}
