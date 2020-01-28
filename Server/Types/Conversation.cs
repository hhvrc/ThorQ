using LiteDB;
using System;
using System.Collections.Generic;
using System.Text;

namespace Server
{
	public class Conversation
	{
		[BsonId]
		public Guid id = Guid.NewGuid();
		public String name = "";
		public List<Guid> members = new List<Guid>();
		public List<Message> messages = new List<Message>();
	}
}
