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
	public static class FriendRequest_RequestHandler
	{
		public static void Dispatch(RuntimeUser thisUser, HostConnection client, RequestMethod method, Guid requestId, string payload)
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
		static void Get(RuntimeUser thisUser, HostConnection client, Guid requestId, string payload)
		{
			Program.userAPI.GetById(thisUser.Id, (dbUser) =>
			{
				Response messageObject = new Response()
				{
					code = ResponseCode.OK,
					type = ResponseType.FRIEND_REQUEST_LIST,
					requestId = requestId,
					payload = JsonConvert.SerializeObject(dbUser.FriendRequests)
				};

				client.SendEncrypted(Encoding.UTF8.GetBytes(JsonConvert.SerializeObject(messageObject)));
			},
			(err) =>
			{
				Console.WriteLine($"FriendRequest_RequestHandler.Get: {err}");
				Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, "Error getting friendslist");
			});
		}
		static void Post(RuntimeUser thisUser, HostConnection client, Guid requestId, string payload)
		{
			Program.userAPI.AddFriendRequest(thisUser.Id, payload, (friendRequestId, targetUserId) =>
			{
				Response messageObject = new Response()
				{
					code = ResponseCode.OK,
					type = ResponseType.FRIEND_REQUEST,
					requestId = requestId,
					payload = friendRequestId.ToString()
				};

				if (Program.initializedUsers.TryGetValue(targetUserId, out var runtimeUser))
					runtimeUser.SendMessage(messageObject.Serialize());

				Program.SimpleClientResponse(client, requestId, ResponseCode.OK, "Friendrequest sent");
			},
			(err) =>
			{
				Console.WriteLine($"FriendRequest_RequestHandler.Post: {err}");
				// To keep anonymity, dont respond differently
				Program.SimpleClientResponse(client, requestId, ResponseCode.OK, "Friendrequest sent");
			});
		}
		static void Accept(RuntimeUser thisUser, HostConnection client, Guid requestId, string payload)
		{
			// TODO implement me
			throw new NotImplementedException($"FriendRequest_RequestHandler.Accept({thisUser},{client},{requestId},{payload})");
		}
		static void Deny(RuntimeUser thisUser, HostConnection client, Guid requestId, string payload)
		{
			// TODO implement me
			throw new NotImplementedException($"FriendRequest_RequestHandler.Deny({thisUser},{client},{requestId},{payload})");
		}
	}
}
