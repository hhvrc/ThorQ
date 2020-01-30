using Newtonsoft.Json;
using System;
using System.Net;

namespace CollarLib
{
	[Serializable]
	public struct Request
	{
		public Guid id;
		public RequestType request;
		public RequestMethod method;
		public String payload;

		public String Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static Request Deserialize(String str)
		{
			return JsonConvert.DeserializeObject<Request>(str);
		}
	}
}
