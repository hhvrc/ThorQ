using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CollarLib
{
	[Serializable]
	public struct BlockedUser
	{
		public BlockedUser(Guid blockedId, string blockedUsername)
		{
			id = Guid.NewGuid();
			this.blockedId = blockedId;
			frozenUsername = blockedUsername;
		}
		public Guid id;
		public Guid blockedId;
		public string frozenUsername;

		public String Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static BlockedUser Deserialize(String str)
		{
			return JsonConvert.DeserializeObject<BlockedUser>(str);
		}
	}
}
