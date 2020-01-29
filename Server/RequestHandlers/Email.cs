using Newtonsoft.Json;
using System;

namespace CollarControl
{
	public static class Email_RequestHandler
	{
		public static void Dispatch(RuntimeUser thisUser, Connection client, RequestMethod method, Guid requestId, string payload)
		{
			switch (method)
			{
				case RequestMethod.GET:
					Get(thisUser, client, requestId, payload);
					break;
				case RequestMethod.SET:
					Set(thisUser, client, requestId, payload);
					break;
				default:
					Program.SimpleClientResponse(client, requestId, ResponseCode.FORBIDDEN, "Not a valid method for this request");
					break;
			}
		}
		static void Get(RuntimeUser thisUser, Connection client, Guid requestId, string payload)
		{
			Program.SimpleClientResponse(client, requestId, ResponseCode.OK, thisUser.Email);
		}
		static void Set(RuntimeUser thisUser, Connection client, Guid requestId, string payload)
		{
			ClientPayloads.EmailSetRequest recovery;
			try
			{
				recovery = ClientPayloads.EmailSetRequest.Deserialize(payload);
			}
			catch (Exception)
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, "Invalid payload");
				return;
			}
			if (thisUser.VerifyPassword(recovery.password))
			{
				Console.WriteLine("Got: " + recovery.newEmail); // DEBUG
				thisUser.Email = recovery.newEmail;
				Program.userAPI.TryUpdate(thisUser);
				Program.SimpleClientResponse(client, requestId, ResponseCode.OK, thisUser.Email);
			}
			else
				Program.SimpleClientResponse(client, requestId, ResponseCode.UNAUTHORIZED, "Invalid password");
		}
	}
}
