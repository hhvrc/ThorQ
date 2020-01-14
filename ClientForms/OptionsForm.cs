using System;
using System.Drawing;
using System.Windows.Forms;

namespace CollarControl
{
	public partial class OptionsForm : Form
	{
		object portlock = new object();
		private ushort m_serverPort = 0;
		private string m_serverHostname = "";

		public string ServerHostname
		{
			get
			{
				lock (m_serverHostname)
					return m_serverHostname;
			}
			set
			{
				if (value != null)
					lock (m_serverHostname)
						m_serverHostname = value;
			}
		}

		public ushort ServerPort
		{
			get
			{
				lock (portlock)
					return m_serverPort;
			}
			set
			{
				lock (portlock)
					m_serverPort = value;
			}
		}

		public OptionsForm()
		{
			InitializeComponent();

			hostnameBox.Text = ServerHostname = Program.CacheGetOrDefault("ServerHostname");

			portBox.Value = ServerPort = Program.CacheTryGet("ServerPort", out string port) ?
				ushort.Parse(port)
				:
				ushort.MinValue;
		}

		private void CheckConnectionButton_Click(object sender, EventArgs e)
		{
			try
			{
				Client client = new Client();
				if (client.Connect(hostnameBox.Text, (ushort)portBox.Value))
					if (client.Authenticate())
						CheckConnectionButton.BackColor = Color.Green;
					else
						CheckConnectionButton.BackColor = Color.Red;
				else
					CheckConnectionButton.BackColor = Color.Red;
			}
			catch (Exception)
			{
				CheckConnectionButton.BackColor = Color.Red;
			}
		}

		private void CancelButton_Click(object sender, EventArgs e)
		{
			hostnameBox.Text = ServerHostname;
			portBox.Value = ServerPort;
			this.Hide();
		}

		private void OkButton_Click(object sender, EventArgs e)
		{
			ServerHostname = hostnameBox.Text;
			ServerPort = (ushort)portBox.Value;

			Program.CacheUpsert("ServerHostname", ServerHostname);
			Program.CacheUpsert("ServerPort", ServerPort.ToString());

			this.Hide();
		}

		private void portBox_ValueChanged(object sender, EventArgs e)
		{
			CheckConnectionButton.BackColor = Color.White;
		}

		private void hostnameBox_TextChanged(object sender, EventArgs e)
		{
			CheckConnectionButton.BackColor = Color.White;
		}
	}
}
