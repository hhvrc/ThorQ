using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Server
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
			throw new NotImplementedException();
		}
		static void Accept(RuntimeUser thisUser, Connection client, Guid requestId, string payload)
		{
			throw new NotImplementedException();
		}
		static void Deny(RuntimeUser thisUser, Connection client, Guid requestId, string payload)
		{
			throw new NotImplementedException();
		}

	}
}
