using System;
using System.Windows.Forms;

namespace ThorQ
{
	public partial class LoginForm : Form
	{
		public LoginForm()
		{
			InitializeComponent();
			this.CenterToScreen();
		}


		private void LoginButton_Click(object sender, EventArgs e)
		{
			if (Connection.IsConnected)
				return;

			if (Connection.Connect())
			{
				Console.WriteLine("Socket connected to {0}:{1}", Connection.ServerHostname, Connection.ServerPort);
				AppState.QueueWindow(AppState.WindowType.Main);
				this.Close();
			}
			else
			{

			}
		}

		private void RegistrationButton_Click(object sender, EventArgs e)
		{
			AppState.QueueWindow(AppState.WindowType.Register);
			this.Close();
		}

		private void OptionsButton_Click(object sender, EventArgs e)
		{
			AppState.QueueWindow(AppState.WindowType.Options);
			this.Close();
		}

		private void ForgotPasswordButton_Click(object sender, EventArgs e)
		{
			AppState.QueueWindow(AppState.WindowType.Recovery);
			this.Close();
		}

		private void OnClosed(object sender, FormClosedEventArgs e)
		{
			AppState.CloseWindow(AppState.WindowType.Login);
		}
	}
}
