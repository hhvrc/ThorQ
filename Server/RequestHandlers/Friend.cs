using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using HeavenLib.Connectivity;
using CollarLib;

namespace ThorQ
{
	public static class Friend_RequestHandler
	{
		public static void Dispatch(RuntimeUser thisUser, HostConnection client, RequestMethod method, Guid requestId, string payload)
		{
			switch (method)
			{
				case RequestMethod.GET:
					Get(thisUser, client, requestId, payload);
					break;
				case RequestMethod.DELETE:
					Delete(thisUser, client, requestId, payload);
					break;
				default:
					Program.SimpleClientResponse(client, requestId, ResponseCode.FORBIDDEN, "Not a valid method for this request");
					break;
			}
		}
		static void Get(RuntimeUser thisUser, HostConnection client, Guid requestId, string payload)
		{
			// TODO implement me
			throw new NotImplementedException($"Friend_RequestHandler.Get({thisUser},{client},{requestId},{payload})");
		}
		static void Delete(RuntimeUser thisUser, HostConnection client, Guid requestId, string payload)
		{
			Guid targetUserId;
			try
			{
				targetUserId = JsonConvert.DeserializeObject<Guid>(payload);
			}
			catch (Exception)
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, "Invalid payload");
				return;
			}

			Program.userAPI.RemoveFriendship(thisUser.Id, targetUserId, (thisDbUser, targetDbUser) =>
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.OK, "Friend removed");

				var response = new CollarLib.Response
				{
					code = ResponseCode.DELETED,
					payload = targetUserId.ToString(),
					requestId = requestId,
					type = ResponseType.FRIEND
				};
				thisUser.SendMessage(response.Serialize());
				if (Program.initializedUsers.TryGetValue(targetUserId, out var targetUser))
				{
					response.payload = thisUser.Id.ToString();
					response.requestId = Guid.Empty;
					targetUser.SendMessage(response.Serialize());
				}
			},
			(err) =>
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.NOPE, err);
			});
		}
	}
}
