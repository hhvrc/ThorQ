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

		public OptionsForm()
		{
			InitializeComponent();
		}

		private void CheckConnectionButton_Click(object sender, EventArgs e)
		{

		}

		private void CancelButton_Click(object sender, EventArgs e)
		{
			this.Hide();
		}

		private void OkButton_Click(object sender, EventArgs e)
		{
			ServerHostname = hostnameBox.Text;
			ServerPort = (UInt16)portBox.Value;
			this.Hide();
		}

		private void OptionsForm_Shown(object sender, EventArgs e)
		{
			hostnameBox.Text = ServerHostname;
			portBox.Value = ServerPort;
		}
	}
}
