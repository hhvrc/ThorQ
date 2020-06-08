using CollarLib;
using Newtonsoft.Json;
using HeavenLib;
using HeavenLib.Connectivity;
using System;

namespace ThorQ
{
	public static class Recovery_RequestHandler
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
			if (thisUser != null)
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.NOPE, "Already logged in!");
				return;
			}

			CollarLib.ClientPayloads.RecoveryPostRequest recovery;
			try
			{
				recovery = JsonConvert.DeserializeObject<CollarLib.ClientPayloads.RecoveryPostRequest>(payload);
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

			if (Program.initializedUsers.TryGetValue(thisUser.Id, out RuntimeUser user))
			{
				if (recovery.token == "")
				{
					user.SendPasswordResetToken(() => { }, (err) =>
					{
						Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, err);
					});
				}
				else
				{
					if (user.VerifyPasswordResetToken(recovery.token))
					{
						Program.userAPI.SetPassword(user.Id, recovery.newPassword, () =>
						{
							Program.SimpleClientResponse(client, requestId, ResponseCode.OK, "Password set!");
						}, (err) =>
						{
							Console.WriteLine($"Failed to set password: {err}");
						});
					}
					else
					{
						Program.SimpleClientResponse(client, requestId, ResponseCode.NOPE, "Code is invalid/expired!");
					}
				}
			}
			else
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.OK, "Recovery password sent!");
			}
		}
	}
}
