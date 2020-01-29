using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CollarControl
{
	[Serializable]
	public struct Message
	{
		public Guid id;
		public Guid usrId;
		public DateTime utcTime;
		public String content;

		public String Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static Message Deserialize(String str)
		{
			return JsonConvert.DeserializeObject<Message>(str);
		}
	}
}
