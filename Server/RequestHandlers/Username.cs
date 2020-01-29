using System;

namespace CollarControl
{
	class Username_RequestHandler
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
			Program.SimpleClientResponse(client, requestId, ResponseCode.OK, thisUser.Username);
		}
		static void Set(RuntimeUser thisUser, Connection client, Guid requestId, string payload)
		{
			if (string.IsNullOrWhiteSpace(payload))
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.INVALID_PARAMS, "Username cannot be empty");
				return;
			}
			Console.WriteLine("Got: " + payload); // DEBUG
			thisUser.Username = payload;
			Program.userAPI.TryUpdate(thisUser);
			Program.SimpleClientResponse(client, requestId, ResponseCode.OK, thisUser.Username);
		}
	}
}
