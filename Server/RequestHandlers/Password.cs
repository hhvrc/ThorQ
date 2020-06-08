using System;
using HeavenLib.Connectivity;
using CollarLib;

namespace ThorQ
{
	class Password_RequestHandler
	{
		public static void Dispatch(RuntimeUser thisUser, HostConnection client, RequestMethod method, Guid requestId, string payload)
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
		static void Post(RuntimeUser thisUser, HostConnection client, Guid requestId, string payload)
		{
			CollarLib.ClientPayloads.PasswordSetRequest recovery;
			try
			{
				recovery = CollarLib.ClientPayloads.PasswordSetRequest.Deserialize(payload);
			}
			catch (Exception)
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, "Invalid payload");
				return;
			}

			Program.userAPI.GetById(thisUser.Id, (dbUser) =>
			{
				if (dbUser.VerifyPassword(recovery.oldPassword))
				{
					Console.WriteLine("Got: " + recovery.newPassword); // DEBUG
					Program.userAPI.SetPassword(thisUser.Id, recovery.newPassword, () =>
					{
						Program.SimpleClientResponse(client, requestId, ResponseCode.OK, "Password set!");
					},
					(err) =>
					{
						Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, err);
					});
				}
				Program.SimpleClientResponse(client, requestId, ResponseCode.UNAUTHORIZED, "Invalid password");
			},
			(err) =>
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, err);
			});
		}
	}
}
