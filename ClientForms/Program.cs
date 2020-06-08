using System;
using System.Windows.Forms;

namespace ThorQ
{
	static class Program
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main()
		{
			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);

			AppState.QueueWindow(AppState.WindowType.Login);

			Form activeForm = null;
			while (!AppState.IsExiting())
			{
				switch (AppState.PendingWindow())
				{
					case AppState.WindowType.Login:
						activeForm = new LoginForm();
						break;
					case AppState.WindowType.Main:
						activeForm = new MainForm();
						break;
					case AppState.WindowType.Options:
						activeForm = new OptionsForm();
						break;
					case AppState.WindowType.Recovery:
						activeForm = new RecoveryForm();
						break;
					case AppState.WindowType.Register:
						activeForm = new RegisterForm();
						break;
				}
				Application.Run(activeForm);
				activeForm.Dispose();
			}

			AppState.Cleanup();
			Connection.Disconnect();
		}
	}
}
