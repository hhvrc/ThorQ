using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ThorQ
{
	public partial class RecoveryForm : Form
	{
		public RecoveryForm()
		{
			InitializeComponent();
			this.CenterToScreen();
		}

		private void button1_Click(object sender, EventArgs e)
		{
			this.Close();
		}

		private void OnClosed(object sender, FormClosedEventArgs e)
		{
			AppState.CloseWindow(AppState.WindowType.Recovery);
		}
	}
}
