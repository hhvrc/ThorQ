using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CollarControl
{
	public static class FriendRequest_RequestHandler
	{
		public static void Dispatch(RuntimeUser thisUser, Connection client, RequestMethod method, Guid requestId, string payload)
		{
			// Get/Set username
			switch (method)
			{
				case RequestMethod.GET:
					Get(thisUser, client, requestId, payload);
					break;
				case RequestMethod.POST:
					Post(thisUser, client, requestId, payload);
					break;
				case RequestMethod.ACCEPT:
					Accept(thisUser, client, requestId, payload);
					break;
				case RequestMethod.DENY:
					Deny(thisUser, client, requestId, payload);
					break;
				default:
					Program.SimpleClientResponse(client, requestId, ResponseCode.FORBIDDEN, "Not a valid method for this request");
					break;
			}
		}
		static void Get(RuntimeUser thisUser, Connection client, Guid requestId, string payload)
		{
			ServerPackage messageObject = new ServerPackage()
			{
				code = ResponseCode.OK,
				type = ResponseDataType.FRIEND_REQUEST_LIST,
				requestId = requestId,
				payload = JsonConvert.SerializeObject(thisUser.FriendRequests),
			};

			client.SendMessage(JsonConvert.SerializeObject(messageObject));
		}
		static void Post(RuntimeUser thisUser, Connection client, Guid requestId, string payload)
		{
			RuntimeUser thatUser = Program.userAPI.GetByName(payload);
			if (thatUser != null)
			{
				// To keep anonymity, dont respond differently
				Program.SimpleClientResponse(client, requestId, ResponseCode.OK, "Friendrequest sent");
				return;
			}

			if (thatUser.HasBlocked(thisUser.Id))
			{
				// To keep anonymity, dont respond differently
				Program.SimpleClientResponse(client, requestId, ResponseCode.OK, "Friendrequest sent");
				return;
			}

			if (thatUser.IsFriendsWith(thisUser.Id))
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.NOPE, "Already friends");
				return;
			}
			FriendRequest req = new FriendRequest(thisUser.Id);
			thatUser.FriendRequests.Add(req);
			Program.userAPI.TryUpdate(thatUser);

			ServerPackage messageObject = new ServerPackage()
			{
				code = ResponseCode.OK,
				type = ResponseDataType.FRIEND_REQUEST,
				requestId = requestId,
				payload = req.Serialize(),
			};

			thatUser.SendMessage(messageObject.Serialize());
			Program.SimpleClientResponse(client, requestId, ResponseCode.OK, "Friendrequest sent");
		}
		static void Accept(RuntimeUser thisUser, Connection client, Guid requestId, string payload)
		{
			// TODO implement me
			throw new NotImplementedException($"FriendRequest_RequestHandler.Accept({thisUser},{client},{requestId},{payload})");
		}
		static void Deny(RuntimeUser thisUser, Connection client, Guid requestId, string payload)
		{
			// TODO implement me
			throw new NotImplementedException($"FriendRequest_RequestHandler.Deny({thisUser},{client},{requestId},{payload})");
		}
	}
}
