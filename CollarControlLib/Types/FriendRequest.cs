using LiteDB;
using System;

namespace CollarLib
{
	[Serializable]
	public struct FriendRequest
	{
		public FriendRequest(Guid fromUserId)
		{
			id = Guid.NewGuid();
			senderId = fromUserId;
		}
		[BsonId]
		public Guid id { get; set; }
		public Guid senderId { get; set; }
	}
}
