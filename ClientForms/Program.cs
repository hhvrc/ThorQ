using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Windows.Forms;

namespace CollarControl
{
	static class Program
	{
		static private Dictionary<string, string> m_appCache = null;

		public static void CacheUpsert(string key, string value)
		{
			lock (m_appCache)
			{
				bool updated = false;
				if (m_appCache.TryGetValue(key, out string oldvalue))
				{
					if (oldvalue != value)
					{
						updated = true;
						m_appCache[key] = value;
					}
				}
				else
				{
					updated = true;
					m_appCache.Add(key, value);
				}
				if (updated)
					File.WriteAllText("cache.txt", JsonConvert.SerializeObject(m_appCache));
			}
		}
		public static bool CacheTryGet(string key, out string value)
		{
			lock (m_appCache)
				return m_appCache.TryGetValue(key, out value);
		}
		public static string CacheGetOrDefault(string key)
		{
			if (CacheTryGet(key, out string value))
				return value;
			return "";
		}

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main()
		{
			m_appCache = File.Exists("cache.txt") ?
				JsonConvert.DeserializeObject<Dictionary<string, string>>(File.ReadAllText("cache.txt", System.Text.Encoding.UTF8))
				:
				new Dictionary<string, string>();

			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
			LoginForm form = new LoginForm();
			Application.Run(form);
			form.Hide();
			form.Close();
			form.Dispose();
		}
	}
}
