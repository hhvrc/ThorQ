using Newtonsoft.Json;
using System;
using HeavenLib.Connectivity;
using CollarLib;

namespace ThorQ
{
	public static class Email_RequestHandler
	{
		public static void Dispatch(RuntimeUser thisUser, HostConnection client, RequestMethod method, Guid requestId, string payload)
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
		static void Get(RuntimeUser thisUser, HostConnection client, Guid requestId, string payload)
		{
			Program.userAPI.GetById(thisUser.Id, (dbUser)=>
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.OK, dbUser.Email);
			},
			(err)=>
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, "Server error!");
			});

		}
		static void Set(RuntimeUser thisUser, HostConnection client, Guid requestId, string payload)
		{
			CollarLib.ClientPayloads.EmailSetRequest recovery;
			try
			{
				recovery = CollarLib.ClientPayloads.EmailSetRequest.Deserialize(payload);
			}
			catch (Exception)
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, "Invalid payload");
				return;
			}
			
			Program.userAPI.GetById(thisUser.Id, (dbUser) =>
			{
				if (dbUser.VerifyPassword(recovery.password))
				{
					Console.WriteLine("Got: " + recovery.newEmail); // DEBUG
					dbUser.Email = recovery.newEmail;
					Program.userAPI.SetEmail(thisUser.Id, recovery.newEmail, () =>
					{
						Program.SimpleClientResponse(client, requestId, ResponseCode.OK, "Email changed!");
					},
					(err) =>
					{
						Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, "Server error!");
					});
				}
				else
				{
					Program.SimpleClientResponse(client, requestId, ResponseCode.UNAUTHORIZED, "Invalid password");
				}
			},
			(err) =>
			{

			});

			
		}
	}
}
