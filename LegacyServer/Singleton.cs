using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace ThorQ
{
	public static class Singleton
	{
		public static Dictionary<string, string> users = new Dictionary<string, string>();
		public static List<Input> inputs = new List<Input>();

		public static void AddInput(UserLogin login, string message)
		{
			string date = DateTime.UtcNow.ToString("yyyy-MM-ddTHH:mm:ss.fff");

			string sessionId = "";

			lock (users)
			{
				if (!users.ContainsKey(login.key))
				{
					sessionId = Crypto.EncryptWithSha256(login.key + "uihjeruihler89780t2378gq" + date);

					users.Add(login.key, sessionId);
				}
				else
				{
					sessionId = users[login.key];
				}
			}

			string signature = Crypto.SignMessage((login.username + message + date), login.key);

			Input input = new Input()
			{
				Signature = signature,
				SessionId = sessionId,
				Username = login.username,
				Message = message,
				TimeStamp = date
			};

			lock (inputs)
			{
				inputs.Insert(0, input);
			}
		}
	}
}