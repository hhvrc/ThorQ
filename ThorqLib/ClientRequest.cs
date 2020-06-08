using Newtonsoft.Json;
using System;
using System.Net;

namespace CollarLib
{
	[Serializable]
	public struct ClientRequest
	{
		public Guid Id;
		public RequestType Request;
		public RequestMethod Method;
		public String Payload;

		public String Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static ClientRequest Deserialize(String str)
		{
			return JsonConvert.DeserializeObject<ClientRequest>(str);
		}
	}
}
