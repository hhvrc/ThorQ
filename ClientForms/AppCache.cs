using Newtonsoft.Json;
using System.Collections.Generic;
using System.IO;

namespace ThorQ
{
	public static class AppCache
	{
		static private Dictionary<string, string> appCache = File.Exists("cache.txt") ?
				   JsonConvert.DeserializeObject<Dictionary<string, string>>(
					   File.ReadAllText("cache.txt", System.Text.Encoding.UTF8))
				   :
				   new Dictionary<string, string>();
		public static void Upsert(string key, string value)
		{
			lock (appCache)
			{
				bool updated = false;
				if (appCache.TryGetValue(key, out string oldvalue))
				{
					if (oldvalue != value)
					{
						updated = true;
						appCache[key] = value;
					}
				}
				else
				{
					updated = true;
					appCache.Add(key, value);
				}
				if (updated)
					File.WriteAllText("cache.txt", JsonConvert.SerializeObject(appCache, Formatting.Indented));
			}
		}
		public static bool TryGet(string key, out string value)
		{
			lock (appCache)
				return appCache.TryGetValue(key, out value);
		}
		public static string GetOrDefault(string key)
		{
			if (TryGet(key, out string value))
				return value;
			return "";
		}
	}
}
