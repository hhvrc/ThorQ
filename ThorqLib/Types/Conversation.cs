using System;
using System.Collections.Generic;
using System.Text;

namespace CollarLib
{
	public struct Conversation
	{
		public Conversation(Guid id, String name, List<Guid> members)
		{
			Id = id;
			Name = name;
			Members = members;
		}
		public Guid Id { get; set; }
		public String Name { get; set; }
		public List<Guid> Members { get; set; }
	}
}
