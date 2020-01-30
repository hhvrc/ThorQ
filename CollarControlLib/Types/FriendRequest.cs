using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CollarLib
{
	[Serializable]
	public struct FriendRequest
	{
		public FriendRequest(Guid from)
		{
			id = Guid.NewGuid();
			userFrom = from;
		}
		public Guid id;
		public Guid userFrom;

		public String Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static FriendRequest Deserialize(String str)
		{
			return JsonConvert.DeserializeObject<FriendRequest>(str);
		}
	}
}
