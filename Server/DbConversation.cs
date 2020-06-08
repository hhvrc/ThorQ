using System;
using System.Collections.Generic;
using System.Text;

namespace ThorQ
{
	public class DbConversation
	{
		public DbConversation(){}
		public DbConversation(Guid id, String name, List<Guid> members, List<CollarLib.ConvMessage> messages)
		{
			Id = id;
			Name = name;
			Members = members;
			Messages = messages;
		}
		public Guid Id { get; set; }
		public String Name { get; set; }
		public List<Guid> Members { get; set; }
		public List<CollarLib.ConvMessage> Messages { get; set; }
	}
}
