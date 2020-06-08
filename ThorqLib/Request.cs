using Newtonsoft.Json;
using System;
using System.Net;

namespace CollarLib
{
	[Serializable]
	public struct Request
	{
		public RequestType type;
		public String Payload;

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
