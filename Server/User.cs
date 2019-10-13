using LiteDB;
using System;
using System.Collections.Generic;

namespace CollarControl
{
	class User
	{
		private object _idLock = new object();
		private Guid _id = Guid.Empty;
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
		public String _username;
		public String Username
		{
			get
			{
				lock (_username)
					return _username;
			}
			set
			{
				lock (_username)
				{
					_username = value;
				}
			}
		}
		public String _email;
		public String Email
		{
			get
			{
				lock (_email)
					return _email;
			}
			set
			{
				lock (_email)
					_email = value.ToLower();
			}
		}

		private String _passwordHash;
		private List<Guid> _friends;
		private List<RequestIDpair> _friendRequests;
		private List<BlockedUser> _blockedUsers;
		[BsonIgnore]
		private List<Connection> _connections = new List<Connection>();
		[BsonIgnore]
		public event Action<Guid, string> OnMessageReceived;

		private void HandleMessage(Connection connection, string message)
		{
			connection.SendMessage(message + "Ack."); // DEBUG
		}

		public bool AddConnection(Connection connection)
		{
			// Do NOT lock one object inside another
			Guid thisId = Id;

			lock (_connections)
			{
				if (!_connections.Contains(connection))
				{
					connection.Id = thisId;
					_connections.Add(connection);
					connection.OnMessageReceived += HandleMessage;
					connection.OnClientDisconnected += DisconnectHandler;
					return true;
				}
				return false;
			}
		}

		public bool HasConnection(Connection connection)
		{
			lock (_connections)
			{
				return _connections.Contains(connection);
			}
		}

		public void RemoveConnection(Connection connection)
		{
			lock (_connections)
			{
				_connections.Remove(connection);
			}
		}

		public void SendMessage(string message)
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
			lock (_connections)
			{
				_connections.Remove(connection);
			}
		}

		public void SetPassword(string password)
		{
			lock (_passwordHash)
			{
				_passwordHash = BCrypt.Net.BCrypt.EnhancedHashPassword(password, BCrypt.Net.HashType.SHA512, 13);
			}
		}


		public bool VerifyPassword(string password)
		{
			lock (_passwordHash)
			{
				return BCrypt.Net.BCrypt.EnhancedVerify(password, _passwordHash, BCrypt.Net.HashType.SHA512);
			}
		}

		public bool HasBlocked(Guid userID)
		{
			lock (_blockedUsers)
			{
				return _blockedUsers.Exists(u => u.UserID == userID);
			}
		}

		public void Block(Guid userID, string username)
		{
			lock (_blockedUsers)
			{
				_blockedUsers.Add(new BlockedUser(userID, username));
			}
		}

		public void Unblock(Guid userID)
		{
			lock (_blockedUsers)
			{
				_blockedUsers.RemoveAll(u => u.UserID == userID);
			}
		}

		public void Unblock(string usernameWhenBlocked)
		{
			lock (_blockedUsers)
			{
				_blockedUsers.RemoveAll(u => u.UsernameWhenBlocked == usernameWhenBlocked);
			}
		}

		public void ClearBlocks()
		{
			lock (_blockedUsers)
			{
				_blockedUsers.Clear();
			}
		}

		public bool HasFriendRequestFromUser(Guid userID)
		{
			lock (_friendRequests)
			{
				return _friendRequests.Exists(r => r.UserID == userID);
			}
		}

		public void AddFriendRequestFromUser(Guid userID)
		{
			lock (_friendRequests)
			{
				_friendRequests.Add(new RequestIDpair(userID));
			}
			// TODO Notify this user
		}

		public void AcceptFriendRequest(Guid requestID)
		{
			lock (_friendRequests)
			{
				// TODO remove request from this user
			}
			lock (_friends)
			{
				// TODO add friend to this user
				// TODO add friend to other user
			}
			// TODO notify users
		}

		public void DenyFriendRequest(Guid requestID)
		{
			lock (_friendRequests)
			{
				// TODO remove request from this user
			}
			// TODO notify this user
		}

		public void ClearFriendRequests()
		{
			lock (_friendRequests)
			{
				_friendRequests.Clear();
			}
		}

		private class RequestIDpair
		{
			public RequestIDpair(Guid userID)
			{
				_requestID = Guid.NewGuid();
				this._userID = userID;
			}
			private object _ridLock = new object();
			private Guid _requestID;
			public Guid RequestID
			{
				get
				{
					lock (_ridLock)
					{
						return _requestID;
					}
				}
				set
				{
					lock (_ridLock)
					{
						_requestID = value;
					}
				}
			}
			private object _uidLock = new object();
			private Guid _userID;
			public Guid UserID
			{
				get
				{
					lock (_uidLock)
					{
						return _userID;
					}
				}
				set
				{
					lock (_uidLock)
					{
						_userID = value;
					}
				}
			}
		}

		private class BlockedUser
		{
			public BlockedUser(Guid userID, string username)
			{
				this._userID = userID;
				this._usernameWhenBlocked = username;
			}
			private object _uidLock = new object();
			private Guid _userID;
			public Guid UserID
			{
				get
				{
					lock (_uidLock)
					{
						return _userID;
					}
				}
				set
				{
					lock (_uidLock)
					{
						_userID = value;
					}
				}
			}
			private string _usernameWhenBlocked;
			public String UsernameWhenBlocked
			{
				get
				{
					lock (_usernameWhenBlocked)
					{
						return _usernameWhenBlocked;
					}
				}
				set
				{
					lock (_usernameWhenBlocked)
					{
						_usernameWhenBlocked = value;
					}
				}
			}
		}

		public static implicit operator Guid(User user)
		{
			return user.Id;
		}
	}
}
