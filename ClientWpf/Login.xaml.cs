using System;
using System.IO.Ports;
using System.Net;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Threading;

namespace ThorQ
{
	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class LoginWindow : Window
	{
		MainWindow mainWindow = new MainWindow();
		bool connectionOk = false;

		public LoginWindow()
		{
			InitializeComponent();

			this.Closing += LoginWindow_Closing;
			
			// MainWindow
			mainWindow.Closing += MainWindow_Closing;
			mainWindow.IsVisibleChanged += MainWindow_IsVisibleChanged;

			// InfoPopup
			InfoPopupOkButton.Click += PopupOkButton_Click;

			// Login tab
			LoginButton.Click += LoginButton_Click;
			DisplaynameInput.TextChanged += LoginInputsValidator;

			// Collar region
			CollarConnectMenuToggleCheckBox.Click += CollarConnectCheckbutton_Click;
			var portName = SerialConnection.PortName;
			if (!String.IsNullOrWhiteSpace(portName))
			{
				CollarConnectPortSelector.ItemsSource = new String[] { portName };
				CollarConnectPortSelector.SelectedIndex = 0;
				CollarConnectTestButton.IsEnabled = true;
			}
			CollarConnectPortSelector.DropDownOpened += CollarConnectPortSelector_DropDownOpened;
			CollarConnectPortSelector.DropDownClosed += CollarConnectPortSelector_DropDownClosed;
			CollarConnectPortSelector.SelectionChanged += CollarConnectPortSelector_SelectionChanged;
			CollarConnectTestButton.PreviewMouseUp += CollarConnectTestButton_MouseUp;
			CollarConnectTestButton.PreviewMouseDown += CollarConnectTestButton_MouseDown;

			// Connection setup
			Connection.ServerMessageReceived += Connection_ServerMessageReceived;
			connectionOk = Connection.Connect();
		}
		public void RemoveEvents()
		{
			this.Closing -= LoginWindow_Closing;
			mainWindow.Closing -= MainWindow_Closing;
		}
		#region WindowHandling
		private void MainWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
		{
			RemoveEvents();
			this.Close();
		}
		private void LoginWindow_Closing(object sender, EventArgs e)
		{
			Connection.Disconnect();
			RemoveEvents();
			mainWindow.Close();
		}
		private void MainWindow_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
		{
			if (mainWindow.IsVisible)
				this.Hide();
			else if (!mainWindow.IsEnabled)
				this.Show();
		}
		#endregion
		#region MessageHandlers
		private void Connection_ServerMessageReceived(CollarLib.Response resp)
		{
			try
			{
				switch (resp.type)
				{
					case CollarLib.ResponseType.Ping:
						break;
					case CollarLib.ResponseType.Ok:
						break;
					case CollarLib.ResponseType.Key:
						DispatcherOperation op = Dispatcher.BeginInvoke((Action)(() =>
						{
							mainWindow.key = resp.payload;
							mainWindow.displayName = DisplaynameInput.Text;

							mainWindow.Show();
						}));
						break;
					case CollarLib.ResponseType.Request:
						mainWindow.ShowIncomingRequestPopup(resp.payload);
						break;
					case CollarLib.ResponseType.P2PInfo:
						var split = resp.payload.Split(':');
						if (split.Length == 2 && HeavenLib.Connectivity.Utils.TryGetIPAddress(split[0], out var ip) && int.TryParse(split[1], out var port))
							mainWindow.ConnectionEstablished(ip, port);
						ShowPopup("Control accepted... Remote address: " + resp.payload);
						break;
					case CollarLib.ResponseType.Denied:
						ShowPopup("Control denied");
						break;
					case CollarLib.ResponseType.Error:
						ShowPopup("Server error: " + resp.payload);
						break;
					default:
						break;
				}
			}
			catch (Exception ex)
			{
				Console.WriteLine(ex.Message);
				return;
			}
		}
		#endregion
		#region PopupWindow
		private void ShowPopup(String popupMsg)
		{
			DispatcherOperation op = Dispatcher.BeginInvoke((Action)(() =>
			{
				InfoPopupText.Text = popupMsg;
				InfoPopup.Width = InfoPopupText.Width;
				InfoPopup.IsOpen = true;
			}));
		}
		private void PopupOkButton_Click(object sender, RoutedEventArgs e)
		{
			this.IsEnabled = true;
			InfoPopup.IsOpen = false;
		}
		#endregion
		#region LoginWindow
		private void LoginButton_Click(object sender, RoutedEventArgs e)
		{
			this.IsEnabled = false;
			if (!Connection.Connect())
			{
				ShowPopup("Cant connect to server!");
				this.IsEnabled = true;
				return;
			}

			if (String.IsNullOrWhiteSpace(DisplaynameInput.Text))
			{
				this.IsEnabled = true;
				return;
			}

			Connection.SendMessage(DisplaynameInput.Text, CollarLib.RequestType.Register);
		}
		private void LoginInputsValidator(object sender, RoutedEventArgs e)
		{
			if (String.IsNullOrWhiteSpace(DisplaynameInput.Text))
			{
				LoginButton.IsEnabled = false;
				loginErrorTextbox.Text = "Username cannot be empty!";
				loginErrorTextbox.Visibility = Visibility.Visible;
				return;
			}
			loginErrorTextbox.Visibility = Visibility.Hidden;
			LoginButton.IsEnabled = true;
		}
		#endregion
		#region CollarConnectRegion
		private void CollarConnectCheckbutton_Click(object sender, RoutedEventArgs e)
		{
			SerialBox.IsEnabled = CollarConnectMenuToggleCheckBox.IsChecked ?? false;
		}
		private void CollarConnectPortSelector_DropDownOpened(object sender, EventArgs e)
		{
			CollarConnectPortSelector.ItemsSource = SerialPort.GetPortNames();
		}
		private void CollarConnectPortSelector_DropDownClosed(object sender, EventArgs e)
		{
			CollarConnectTestButton.IsEnabled = !String.IsNullOrWhiteSpace((String)CollarConnectPortSelector.SelectedItem);
		}
		private void CollarConnectPortSelector_SelectionChanged(object sender, SelectionChangedEventArgs e)
		{
			Console.WriteLine();
			SerialConnection.PortName = (String)CollarConnectPortSelector.SelectedItem;
		}
		private object l_test = new object();
		private bool m_test = false;
		private bool Testing()
		{
			bool ret;
			lock (l_test)
				ret = m_test;
			return ret;
		}
		private void CollarConnectTestButton_MouseDown(object sender, RoutedEventArgs e)
		{
			lock (l_test)
				m_test = true;
			Task.Run(() =>
			{
				SerialConnection.Open();
				while (Testing())
				{
					SerialConnection.SendCommand('W');
					Thread.Sleep(10);
				}
				SerialConnection.Close();
			});
		}
		private void CollarConnectTestButton_MouseUp(object sender, RoutedEventArgs e)
		{
			lock (l_test)
				m_test = false;
		}
		#endregion
	}
}
