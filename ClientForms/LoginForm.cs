using System;
using System.Threading;
using System.Windows.Forms;

namespace CollarControl
{
	public partial class LoginForm : Form
	{
		private static Client client = null;
		private static MainForm mainForm = null;
		private static OptionsForm optionsForm = null;
		private static RegisterForm registerForm = null;
		private static RecoveryForm recoveryForm = null;

		public LoginForm()
		{
			InitializeComponent();

			mainForm = new MainForm();
			mainForm.VisibleChanged += MainForm_VisibleChanged;

			optionsForm = new OptionsForm();
			optionsForm.VisibleChanged += OptionsForm_VisibleChanged;

			registerForm = new RegisterForm();
			registerForm.VisibleChanged += RegisterForm_VisibleChanged;

			recoveryForm = new RecoveryForm();
			recoveryForm.VisibleChanged += RecoveryForm_VisibleChanged;
		}

		private void RecoveryForm_VisibleChanged(object sender, EventArgs e)
		{
			this.Enabled = !recoveryForm.Visible;
			this.Visible = !recoveryForm.Visible;
		}

		private void RegisterForm_VisibleChanged(object sender, EventArgs e)
		{
			this.Enabled = !registerForm.Visible;
			this.Visible = !registerForm.Visible;
		}

		private void MainForm_VisibleChanged(object sender, EventArgs e)
		{
			this.Enabled = !mainForm.Visible;
			this.Visible = !mainForm.Visible;

			if (!mainForm.Visible)
			{
				client.Cleanup();
				client = null;
			}
		}

		private void OptionsForm_VisibleChanged(object sender, EventArgs e)
		{
			this.Enabled = !optionsForm.Visible;
		}

		public new void Dispose()
		{
			try { client.Cleanup(); } catch (Exception) { }

			try { mainForm.Close(); } catch (Exception) { }
			try { mainForm.Dispose(); } catch (Exception) { }

			try { registerForm.Close(); } catch (Exception) { }
			try { registerForm.Dispose(); } catch (Exception) { }

			try { recoveryForm.Close(); } catch (Exception) { }
			try { recoveryForm.Dispose(); } catch (Exception) { }

			try { this.Close(); } catch (Exception) { }

			base.Dispose();
		}

		static void MessageReceivedHandler(Client client, string str)
		{
			Console.WriteLine(str);
		}

		private void DisconnectHandler(Client cli)
		{
			client.Cleanup();
			client = null;
		}

		static void LoginHandler(Client client)
		{
			Console.WriteLine("Logging in...");

			try
			{
				if (client.Authenticate())
				{
					Console.WriteLine("Authenticated!");
				}
				else
				{
					Console.WriteLine("Authentication failed!");
					// TODO show errormessage, back to loginscreen
					return;
				}
			}
			catch (Exception ex)
			{
				Console.WriteLine("Could not authenticate: " + ex.ToString());
				// TODO show errormessage, back to loginscreen
				return;
			}

			client.StartListening();
			mainForm.Show();
		}

		private void LoginButton_Click(object sender, EventArgs e)
		{
			if (client != null)
				return;

			try
			{
				client = new Client();

				client.OnConnected += LoginHandler;
				client.OnDisconnected += DisconnectHandler;
				client.OnMessageReceived += MessageReceivedHandler;

				if (client.Connect(optionsForm.ServerHostname, optionsForm.ServerPort))
					Console.WriteLine("Socket connected to {0}:{1}", optionsForm.ServerHostname, optionsForm.ServerPort);
				else
					Console.WriteLine("Failed to connect!"); // TODO show errormessage, back to loginscreen
			}
			catch (Exception ex)
			{
				Console.WriteLine("Couldn't initialize client: " + ex.Message); // TODO show errormessage, back to loginscreen
			}
		}

		private void LoginForm_Closed(object sender, FormClosedEventArgs e)
		{
		}

		private void RegistrationButton_Click(object sender, EventArgs e)
		{
			registerForm.Show();
		}

		private void OptionsButton_Click(object sender, EventArgs e)
		{
			optionsForm.Show();
		}

		private void ForgotPasswordButton_Click(object sender, EventArgs e)
		{
			recoveryForm.Show();
		}
	}
}
