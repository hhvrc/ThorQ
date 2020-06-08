using System;
using System.Drawing;
using System.Windows.Forms;

namespace ThorQ
{
	public partial class OptionsForm : Form
	{
		public const int WM_NCLBUTTONDOWN = 0xA1;
		public const int HT_CAPTION = 0x2;

		[System.Runtime.InteropServices.DllImport("user32.dll")]
		public static extern int SendMessage(IntPtr hWnd, int Msg, int wParam, int lParam);
		[System.Runtime.InteropServices.DllImport("user32.dll")]
		public static extern bool ReleaseCapture();

		public OptionsForm()
		{
			InitializeComponent();
			this.CenterToScreen();

			hostnameBox.Text = Connection.ServerHostname;
			portBox.Value = Connection.ServerPort;
		}

		private void CheckConnectionButton_Click(object sender, EventArgs e)
		{
			CheckConnectionButton.BackColor = 
				Connection.TestAddress(hostnameBox.Text, (ushort)portBox.Value)?
				Color.Green
				:
				Color.Red;
		}

		private void CancelButton_Click(object sender, EventArgs e)
		{
			this.Close();
		}

		private void OkButton_Click(object sender, EventArgs e)
		{
			Connection.ServerHostname = hostnameBox.Text;
			Connection.ServerPort = (ushort)portBox.Value;
			this.Close();
		}

		private void portBox_ValueChanged(object sender, EventArgs e)
		{
			CheckConnectionButton.BackColor = Color.White;
		}

		private void hostnameBox_TextChanged(object sender, EventArgs e)
		{
			CheckConnectionButton.BackColor = Color.White;
		}

		private void OptionsForm_MouseDown(object sender, MouseEventArgs e)
		{
			if (e.Button == MouseButtons.Left)
			{
				ReleaseCapture();
				SendMessage(Handle, WM_NCLBUTTONDOWN, HT_CAPTION, 0);
			}
		}

		private void OnClosed(object sender, FormClosedEventArgs e)
		{
			AppState.CloseWindow(AppState.WindowType.Options);
		}
	}
}
