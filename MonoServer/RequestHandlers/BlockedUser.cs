using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CollarControl
{
	public static class BlockedUser_RequestHandler
	{
		public static void Dispatch(RuntimeUser thisUser, Connection client, RequestMethod method, Guid requestId, string payload)
		{
			switch (method)
			{
				case RequestMethod.GET:
					Get(thisUser, client, requestId, payload);
					break;
				case RequestMethod.POST:
					Post(thisUser, client, requestId, payload);
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
			throw new NotImplementedException($"BlockedUser_RequestHandler.Get({thisUser},{client},{requestId},{payload})");
		}
		static void Post(RuntimeUser thisUser, Connection client, Guid requestId, string payload)
		{
			// TODO implement me
			throw new NotImplementedException($"BlockedUser_RequestHandler.Post({thisUser},{client},{requestId},{payload})");
		}
		static void Delete(RuntimeUser thisUser, Connection client, Guid requestId, string payload)
		{
			// TODO implement me
			throw new NotImplementedException($"BlockedUser_RequestHandler.Delete({thisUser},{client},{requestId},{payload})");
		}

	}
}
