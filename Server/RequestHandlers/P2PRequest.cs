using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CollarControl
{
	public static class P2PRequest_RequestHandler
	{
		public static void Dispatch(RuntimeUser thisUser, Connection client, RequestMethod method, Guid requestId, string payload)
		{
			switch (method)
			{
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
		static void Post(RuntimeUser thisUser, Connection client, Guid requestId, string payload)
		{
			// TODO implement me
			throw new NotImplementedException($"P2PRequest_RequestHandler.Post({thisUser},{client},{requestId},{payload})");
		}
		static void Accept(RuntimeUser thisUser, Connection client, Guid requestId, string payload)
		{
			// TODO implement me
			throw new NotImplementedException($"P2PRequest_RequestHandler.Accept({thisUser},{client},{requestId},{payload})");
		}
		static void Deny(RuntimeUser thisUser, Connection client, Guid requestId, string payload)
		{
			// TODO implement me
			throw new NotImplementedException($"P2PRequest_RequestHandler.Deny({thisUser},{client},{requestId},{payload})");
		}

	}
}
