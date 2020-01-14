using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace CollarControl
{
	public partial class OptionsForm : Form
	{
		private UInt16 m_serverPort = 0;
		private String m_serverHostname = "";

		public String ServerHostname
		{
			get { return m_serverHostname; }
			set { m_serverHostname = value; }
		}

		public UInt16 ServerPort
		{
			get { return m_serverPort; }
			set { m_serverPort = value; }
		}

		public OptionsForm(String hostname = "", UInt16 port = 0)
		{
			InitializeComponent();
			hostnameBox.Text = ServerHostname = hostname;
			portBox.Value = ServerPort = port;
		}

		private void CheckConnectionButton_Click(object sender, EventArgs e)
		{
			try
			{
				Client client = new Client();
				if (client.Connect(hostnameBox.Text, (UInt16)portBox.Value))
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
			ServerPort = (UInt16)portBox.Value;
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
