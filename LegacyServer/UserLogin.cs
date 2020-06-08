using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace ThorQ
{
	public class UserLogin
	{
		public string username;
		public string password;

		public string key => Crypto.GetPublicKey(username, password);

		public UserLogin(string username, string password)
		{
			this.username = username;
			this.password = password;
		}

		public bool IsValid()
		{
			return !String.IsNullOrWhiteSpace(username) && !String.IsNullOrEmpty(password) && (key.Length == 64);
		}
	}
}