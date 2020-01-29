using System;

namespace CollarControl
{
	class Password_RequestHandler
	{
		public static void Dispatch(RuntimeUser thisUser, Connection client, RequestMethod method, Guid requestId, string payload)
		{
			switch (method)
			{
				case RequestMethod.POST:
					Post(thisUser, client, requestId, payload);
					break;
				default:
					Program.SimpleClientResponse(client, requestId, ResponseCode.FORBIDDEN, "Not a valid method for this request");
					break;
			}
		}
		static void Post(RuntimeUser thisUser, Connection client, Guid requestId, string payload)
		{
			ClientPayloads.PasswordSetRequest recovery;
			try
			{
				recovery = ClientPayloads.PasswordSetRequest.Deserialize(payload);
			}
			catch (Exception)
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, "Invalid payload");
				return;
			}
			if (thisUser.VerifyPassword(recovery.oldPassword))
			{
				Console.WriteLine("Got: " + recovery.newPassword); // DEBUG
				thisUser.SetPassword(recovery.newPassword);
				Program.userAPI.TryUpdate(thisUser);
				Program.SimpleClientResponse(client, requestId, ResponseCode.OK, "Password set!");
			}
			Program.SimpleClientResponse(client, requestId, ResponseCode.UNAUTHORIZED, "Invalid password");
		}
	}
}
