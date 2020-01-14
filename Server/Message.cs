using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Text;

namespace CollarControl
{
	[Serializable]
	struct Message
	{
		public String Command { get; set; }
		public Dictionary<String, String> Parameters { get; set; }
	}
}
