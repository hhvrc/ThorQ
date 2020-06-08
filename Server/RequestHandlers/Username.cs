using System;
using HeavenLib.Connectivity;
using CollarLib;

namespace ThorQ
{
	class Username_RequestHandler
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
			Program.userAPI.GetById(thisUser.Id, (dbUser) =>
			{
				var messageObject = new CollarLib.Response()
				{
					code = ResponseCode.OK,
					type = ResponseType.USERNAME,
					requestId = requestId,
					payload = dbUser.Username,
				};
				thisUser.SendMessage(messageObject.Serialize());
			},
			(err) =>
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, "Server error");
			});
			Console.WriteLine("Got: " + payload); // DEBUG
		}
		static void Set(RuntimeUser thisUser, HostConnection client, Guid requestId, string payload)
		{
			if (string.IsNullOrWhiteSpace(payload) || payload.Length < 3)
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.INVALID_PARAMS, "Username too short/empty");
				return;
			}
			Program.userAPI.SetUsername(thisUser.Id, payload, () =>
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.OK, "Username set");
			},
			(err) =>
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.OK, "Server error");
			});
		}
	}
}
