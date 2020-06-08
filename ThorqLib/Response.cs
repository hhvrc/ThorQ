using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CollarLib
{
	[Serializable]
	public struct Response
	{
		public ResponseType type;
		public String payload;

		public String Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static Response Deserialize(String str)
		{
			return JsonConvert.DeserializeObject<Response>(str);
		}
	}
}
