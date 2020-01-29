using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CollarControl
{
	public static class Friend_RequestHandler
	{
		public static void Dispatch(RuntimeUser thisUser, Connection client, RequestMethod method, Guid requestId, string payload)
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
		static void Get(RuntimeUser thisUser, Connection client, Guid requestId, string payload)
		{
			// TODO implement me
			throw new NotImplementedException($"Friend_RequestHandler.Get({thisUser},{client},{requestId},{payload})");
		}
		static void Delete(RuntimeUser thisUser, Connection client, Guid requestId, string payload)
		{
			Guid request;
			try
			{
				request = JsonConvert.DeserializeObject<Guid>(payload);
			}
			catch (Exception)
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, "Invalid payload");
				return;
			}

			if (!thisUser.IsFriendsWith(request))
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.UNAUTHORIZED, "Friend not found");
				return;
			}

			RuntimeUser thatUser = Program.userAPI.GetById(request);
			if (thatUser == null)
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, "Server error");
				return;
			}

			// Remove friend from this user
			thatUser.Friends.Remove(thisUser.Id);
			Program.userAPI.TryUpdate(thisUser);

			// Remove friend from other user
			thisUser.Friends.Remove(thatUser.Id);
			Program.userAPI.TryUpdate(thatUser);

			Program.SimpleClientResponse(client, requestId, ResponseCode.OK, "Friend removed");

			thisUser.SendUpdatedFriendsList();
			thatUser.SendUpdatedFriendsList();
		}
	}
}
