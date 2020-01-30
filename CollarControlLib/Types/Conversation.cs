using System;
using System.Collections.Generic;
using System.Text;

namespace CollarLib
{
	public class Conversation
	{
		public Guid id = Guid.NewGuid();
		public String name = "";
		public List<Guid> members = new List<Guid>();
		public List<ConvMessage> messages = new List<ConvMessage>();
	}
}
