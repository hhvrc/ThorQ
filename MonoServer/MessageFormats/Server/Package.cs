using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CollarControl
{
	[Serializable]
	public struct ServerPackage
	{
		public ResponseCode code;
		public ResponseDataType type;
		public Guid requestId; // Will be id of request if message is a response
		public String payload;

		public String Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static ServerPackage Deserialize(String str)
		{
			return JsonConvert.DeserializeObject<ServerPackage>(str);
		}
	}
}
