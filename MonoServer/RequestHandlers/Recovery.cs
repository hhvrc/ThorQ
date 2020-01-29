using Newtonsoft.Json;
using System;

namespace CollarControl
{
	public static class Recovery_RequestHandler
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
			if (thisUser != null)
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.NOPE, "Already logged in!");
				return;
			}

			ClientPayloads.RecoveryPostRequest recovery;
			try
			{
				recovery = JsonConvert.DeserializeObject<ClientPayloads.RecoveryPostRequest>(payload);
			}
			catch (Exception)
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, "Invalid payload");
				return;
			}

			if (!ToolBox.IsValidEmail(recovery.email))
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.OK, "Recovery password sent!");
				return;
			}

			RuntimeUser user = Program.userAPI.GetByEmail(recovery.email);
			if (user != null)
			{
				if (recovery.token == "")
				{
					if (!user.SendPasswordResetToken())
					{
						Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, "Failed to send email!");
						return;
					}
				}
				else
				{
					if (user.VerifyPasswordResetToken(recovery.token))
					{
						user.SetPassword(recovery.newPassword);
						Program.SimpleClientResponse(client, requestId, ResponseCode.OK, "Password set!");
						return;
					}
					Program.SimpleClientResponse(client, requestId, ResponseCode.NOPE, "Code is invalid/expired!");
					return;
				}
			}

			Program.SimpleClientResponse(client, requestId, ResponseCode.OK, "Recovery password sent!");

		}
	}
}
