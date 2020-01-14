using System;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace CollarControl
{
	public partial class LoginForm : Form
	{
		static Client client = null;
		static MainForm mainForm = null;
		static OptionsForm optionsForm = null;
		static RegisterForm registerForm = null;
		static RecoveryForm recoveryForm = null;

		public LoginForm()
		{
			InitializeComponent();

			client = new Client();
			mainForm = new MainForm();
			optionsForm = new OptionsForm();
			registerForm = new RegisterForm();
			recoveryForm = new RecoveryForm();

			client.OnConnected += LoginHandler;
			client.OnDisconnected += DisconnectHandler;
			client.OnMessageReceived += MessageReceivedHandler;

			try
			{
				client.Connect("127.0.0.1", 5001);
				Console.WriteLine("Socket connected to {0}:{1}", "192.168.1.43", 10235);
			}
			catch (Exception ex)
			{
				Console.WriteLine("Couldn't initialize client: " + ex.Message);
			}
		}
		~LoginForm()
		{
			try { client.Cleanup(); } catch (Exception) { }

			try { mainForm.Close(); } catch (Exception) { }
			try { mainForm.Dispose(); } catch (Exception) { }

			try { registerForm.Close(); } catch (Exception) { }
			try { registerForm.Dispose(); } catch (Exception) { }

			try { recoveryForm.Close(); } catch (Exception) { }
			try { recoveryForm.Dispose(); } catch (Exception) { }

			this.Close();
		}

		static void MessageReceivedHandler(Client client, string str)
		{
			Console.WriteLine(str);
		}

		static void DisconnectHandler(Client client)
		{
			client.Cleanup();
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
				}
			}
			catch (Exception ex)
			{
				Console.WriteLine("Could not authenticate: " + ex.ToString());
			}

			client.StartListening();
		}

		private void LoginButton_Click(object sender, EventArgs e)
		{
			// Check if everything is correct withthe server

			this.Hide();
			mainForm.Show();

			Thread.Sleep(1000);

			this.Show();
		}

		private void LoginForm_Closed(object sender, FormClosedEventArgs e)
		{
			client.Cleanup();
			client = null;
		}

		private void RegistrationButton_Click(object sender, EventArgs e)
		{
			registerForm.Show();
		}

		private void LoginForm_Load(object sender, EventArgs e)
		{

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

	class User
	{
		public string name;

		public User(string name)
		{
			this.name = name;
		}
	}

	class Connection
	{
		public User user;
		public Client connection;

		public Connection(User user, Client connection)
		{
			this.user = user;
			this.connection = connection;
		}
	}
}
