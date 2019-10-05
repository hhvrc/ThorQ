using System;
using System.Collections.Generic;
using System.Text;

namespace CollarControl
{
	class User
	{
		public Guid Id { get; set; }
		public String Username { get; set; }
		public String PasswordHash { get; set; }
		public String Email { get; set; }

		public Guid[] Friends { get; set; }
		public Guid[] Blocked { get; set; }
		public Guid[] FriendRequests { get; set; }
	}
}
