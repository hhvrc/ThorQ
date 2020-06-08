using System.Collections.Generic;

namespace ThorQ
{
	public static class AppState
	{
		private static List<WindowType> appStack = new List<WindowType>();

		public enum WindowType
		{
			None,
			Login,
			Main,
			Options,
			Recovery,
			Register,
		}

		public static void QueueWindow(WindowType type)
		{
			lock (appStack)
				appStack.Add(type);
		}
		public static void CloseWindow(WindowType type)
		{
			lock (appStack)
				if (appStack.Count > 0 && appStack[appStack.Count - 1] == type)
					appStack.RemoveAt(appStack.Count - 1);
		}
		public static void ExitApplication()
		{
			lock (appStack)
				appStack.Clear();
		}
		public static bool IsExiting()
		{
			lock (appStack)
				return appStack.Count == 0;
		}
		public static WindowType PendingWindow()
		{
			lock (appStack)
				if (appStack.Count > 0)
					return appStack[appStack.Count - 1];
			return WindowType.None;
		}

		public static Instance instance = null;

		public static string serverHostname = "";

		public static void Cleanup()
		{

		}
	}
}
