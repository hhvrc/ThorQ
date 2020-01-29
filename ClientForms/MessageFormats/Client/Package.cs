using Newtonsoft.Json;
using System;
using System.Net;

namespace CollarControl
{
	[Serializable]
	public struct ClientPackage
	{
		public Guid id;
		public RequestType request;
		public RequestMethod method;
		public String payload;

		public String Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static ClientPackage Deserialize(String str)
		{
			return JsonConvert.DeserializeObject<ClientPackage>(str);
		}
	}
}
