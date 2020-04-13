using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace CollarControl
{
	/// <summary>
	/// Interaction logic for Window1.xaml
	/// </summary>
	public partial class MainWindow : Window
	{
		public String key;
		public String displayName;
		public EndPoint remotePeer = null;
		public HeavenLib.Connectivity.UdpConnection client = new HeavenLib.Connectivity.UdpConnection(true);

		public MainWindow()
		{
			InitializeComponent();

			CopyIdButton.Click += CopyIdButton_Click;
			RequestControlButton.Click += RequestControlButton_Click;
			EndControlButton.Click += EndControlButton_Click;

			WarnButton.PreviewMouseDown += WarnButton_PreviewMouseDown;
			WarnButton.PreviewMouseUp += WarnButton_PreviewMouseUp;
			PunishButton.PreviewMouseDown += PunishButton_PreviewMouseDown;
			PunishButton.PreviewMouseUp += PunishButton_PreviewMouseUp;

			client.OnMessageReceived += Connection_OnMessageReceived;
			IdInputBox.TextChanged += IdInputBox_TextChanged;

			RequestPopupDenyButton.Click += RequestPopupDenyButton_Click;
			RequestPopupAcceptButton.Click += RequestPopupAcceptButton_Click;
		}

		private void IdInputBox_TextChanged(object sender, TextChangedEventArgs e)
		{
			if (String.IsNullOrWhiteSpace(IdInputBox.Text))
			{
				WarnButton.IsEnabled = false;
				PunishButton.IsEnabled = false;
				PunishStrengthSelector.IsEnabled = false;
				RequestControlButton.IsEnabled = false;
				EndControlButton.IsEnabled = false;
				return;
			}
			RequestControlButton.IsEnabled = true;
		}

		private void RequestPopupAcceptButton_Click(object sender, RoutedEventArgs e)
		{
			Connection.SendMessage("", CollarLib.RequestType.Accept);
			RequestPopup.IsOpen = false;
		}

		private void RequestPopupDenyButton_Click(object sender, RoutedEventArgs e)
		{
			Connection.SendMessage("", CollarLib.RequestType.Deny);
			RequestPopup.IsOpen = false;
		}

		public void ShowIncomingRequestPopup(String displayname)
		{
			DispatcherOperation op = Dispatcher.BeginInvoke((Action)(() =>
			{
				RequestPopupText.Text = $"{displayname} is requesting control";
				RequestPopup.Width = InfoPopupText.Width;
				RequestPopup.IsOpen = true;
			}));
		}

		public void ConnectionEstablished(IPAddress address, int port)
		{
			remotePeer = new IPEndPoint(address, port);

			DispatcherOperation op = Dispatcher.BeginInvoke((Action)(() =>
			{
				WarnButton.IsEnabled = true;
				PunishButton.IsEnabled = true;
				PunishStrengthSelector.IsEnabled = true;
				RequestControlButton.IsEnabled = false;
				EndControlButton.IsEnabled = true;
			}));
		}

		private void CopyIdButton_Click(object sender, RoutedEventArgs e)
		{
			if (key != null)
				Clipboard.SetText(key);
		}

		private void RequestControlButton_Click(object sender, RoutedEventArgs e)
		{
			Connection.SendMessage(IdInputBox.Text, CollarLib.RequestType.Request);
		}

		private void EndControlButton_Click(object sender, RoutedEventArgs e)
		{
			remotePeer = null;
		}

		private void Connection_OnMessageReceived(EndPoint ep, byte[] data)
		{
			if (ep == remotePeer)
			{
				SerialConnection.Open();
				SerialConnection.SendCommand((char)data[0]);
				SerialConnection.Close();
			}
		}

		private object l_held = new object();
		private uint m_held = 0;
		private uint Held()
		{
			uint ret;
			lock (l_held)
				ret = m_held;
			return ret;
		}
		private void PunishButton_PreviewMouseUp(object sender, MouseButtonEventArgs e)
		{
			lock (l_held)
				m_held = (uint)(PunishButton.IsPressed ? 2 : WarnButton.IsPressed ? 1 : 0);
		}

		private void PunishButton_PreviewMouseDown(object sender, MouseButtonEventArgs e)
		{
			lock (l_held)
				m_held = (uint)(PunishButton.IsPressed ? 2 : WarnButton.IsPressed ? 1 : 0);
			Task.Run(() =>
			{
				while (true)
				{
					Thread.Sleep(10);
					switch (Held())
					{
						case 1:
							client.SendTo(new byte[] { (byte)'W' }, remotePeer);
							continue;
						case 2:
							client.SendTo(new byte[] { (byte)'P' }, remotePeer);
							continue;
						default:
							break;
					}
				}
			});
		}

		private void WarnButton_PreviewMouseUp(object sender, MouseButtonEventArgs e)
		{
			lock (l_held)
				m_held = (uint)(PunishButton.IsPressed ? 2 : WarnButton.IsPressed ? 1 : 0);
		}

		private void WarnButton_PreviewMouseDown(object sender, MouseButtonEventArgs e)
		{
			lock (l_held)
				m_held = (uint)(PunishButton.IsPressed ? 2 : WarnButton.IsPressed ? 1 : 0);
			Task.Run(() =>
			{
				while (true)
				{
					Thread.Sleep(10);
					switch (Held())
					{
						case 1:
							client.SendTo(new byte[] { (byte)'W' }, remotePeer);
							continue;
						case 2:
							client.SendTo(new byte[] { (byte)'P' }, remotePeer);
							continue;
						default:
							break;
					}
				}
			});
		}
	}
}
