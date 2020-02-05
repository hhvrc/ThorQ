using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace CollarControl
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

			mainWindow.Closing += MainWindow_Closing;
			mainWindow.IsVisibleChanged += MainWindow_IsVisibleChanged;

			Closed += LoginWindow_Closed;
			UsernameInput.TextChanged += UsernameInput_TextChanged;
			PasswordInput.PasswordChanged += PasswordInput_PasswordChanged;
			CollarConnectCheckbutton.Click += CollarConnectCheckbutton_Click;
			ConnectionTestButton.Click += ConnectionTestButton_Click;
			PopupOkButton.Click += PopupOkButton_Click;

			Connection.MessageReceived += Connection_MessageReceived;
			connectionOk = Connection.Connect();
			OptHostname.Text = Connection.ServerHostname;
			OptNetworkPort.Text = Connection.ServerPort.ToString();
		}

		private void ShowPopup(String popupMsg)
		{
			DispatcherOperation op = Dispatcher.BeginInvoke((Action)(() =>
			{
				PopupText.Text = popupMsg;
				Popup1.Width = PopupText.Width;
				Popup1.IsOpen = true;
			}));
		}

		private void Connection_MessageReceived(string obj)
		{
			if (obj == null)
				return;

			CollarLib.Response resp;

			try
			{
				resp = JsonConvert.DeserializeObject<CollarLib.Response>(obj);

				if (resp.code == CollarLib.ResponseCode.NOPE)
				{
					ShowPopup("NOPE: " + resp.payload);
					return;
				}
				else if (resp.code == CollarLib.ResponseCode.FORBIDDEN)
				{
					ShowPopup("FORBIDDEN: " + resp.payload);
					return;
				}
				else if (resp.code == CollarLib.ResponseCode.UNAUTHORIZED)
				{
					ShowPopup("UNAUTHORIZED: " + resp.payload);
					return;
				}
				else if (resp.code == CollarLib.ResponseCode.ERROR)
				{
					ShowPopup("ERROR: " + resp.payload);
					return;
				}

				switch (resp.type)
				{
					case CollarLib.ResponseDataType.NULL:
						break;
					case CollarLib.ResponseDataType.STRING:
						break;
					case CollarLib.ResponseDataType.RPC:
						break;
					case CollarLib.ResponseDataType.P2PR:
						break;
					case CollarLib.ResponseDataType.ACCOUNT:
						/*
						var acc = JsonConvert.DeserializeObject<CollarLib.ServerPayloads.Account>(resp.payload);
						acc.*/
						break;
					case CollarLib.ResponseDataType.BLOCKED_USER:
						break;
					case CollarLib.ResponseDataType.BLOCKED_USER_LIST:
						break;
					case CollarLib.ResponseDataType.FRIEND:
						break;
					case CollarLib.ResponseDataType.FRIEND_LIST:
						break;
					case CollarLib.ResponseDataType.FRIEND_REQUEST:
						break;
					case CollarLib.ResponseDataType.FRIEND_REQUEST_LIST:
						break;
					case CollarLib.ResponseDataType.MESSAGE:
						break;
					case CollarLib.ResponseDataType.MESSAGE_LIST:
						break;
					case CollarLib.ResponseDataType.CONVERSATION:
						break;
					case CollarLib.ResponseDataType.CONVERSATION_LIST:
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

			

			Console.WriteLine(obj);
		}

		private void PopupOkButton_Click(object sender, RoutedEventArgs e)
		{
			this.IsEnabled = true;
			Popup1.IsOpen = false;
		}

		private void ConnectionTestButton_Click(object sender, RoutedEventArgs e)
		{
			PopupText.Text = Connection.TestAddress(OptHostname.Text, ushort.Parse(OptNetworkPort.Text)) ? "Looks ok!" : "Wrong hostname/port";
			this.IsEnabled = false;
			Popup1.IsOpen = true;
		}

		private void CollarConnectCheckbutton_Click(object sender, RoutedEventArgs e)
		{
			SerialBox.IsEnabled = CollarConnectCheckbutton.IsChecked??false;
		}

		public void RemoveEvents()
		{
			Closed -= LoginWindow_Closed;
			mainWindow.Closing -= MainWindow_Closing;
		}

		private void MainWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
		{
			RemoveEvents();
			this.Close();
		}

		private void PasswordInput_PasswordChanged(object sender, RoutedEventArgs e)
		{
			loginErrorTextbox.Visibility = Visibility.Hidden;
			LoginButton.IsEnabled = (!String.IsNullOrWhiteSpace(UsernameInput.Text) && !String.IsNullOrWhiteSpace(PasswordInput.Password));
		}

		private void UsernameInput_TextChanged(object sender, TextChangedEventArgs e)
		{
			loginErrorTextbox.Visibility = Visibility.Hidden;
			LoginButton.IsEnabled = (!String.IsNullOrWhiteSpace(UsernameInput.Text) && !String.IsNullOrWhiteSpace(PasswordInput.Password));
		}

		private void LoginWindow_Closed(object sender, EventArgs e)
		{
			Connection.Disconnect();
			RemoveEvents();
			mainWindow.Close();
		}

		private void MainWindow_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
		{
			if (mainWindow.IsVisible)
				this.Hide();
			else
				this.Show();
		}

		private void OnLoginButtonClicked(object sender, RoutedEventArgs e)
		{
			if (!Connection.Connect())
			{
				ShowPopup("Cant connect to server!");
				return;
			}

			if (String.IsNullOrWhiteSpace(UsernameInput.Text))
			{
				loginErrorTextbox.Text = "Username cannot be empty!";
				loginErrorTextbox.Visibility = Visibility.Visible;
				return;
			}

			if (String.IsNullOrWhiteSpace(PasswordInput.Password))
			{
				loginErrorTextbox.Text = "Password cannot be empty!";
				loginErrorTextbox.Visibility = Visibility.Visible;
				return;
			}

			CollarLib.ClientPayloads.AccountGetRequest payload = new CollarLib.ClientPayloads.AccountGetRequest()
			{
				username = UsernameInput.Text,
				password = PasswordInput.Password,
			};

			CollarLib.Request request = new CollarLib.Request()
			{
				id = Guid.NewGuid(),
				request = CollarLib.RequestType.Account,
				method = CollarLib.RequestMethod.GET,
				payload = payload.Serialize(),
			};

			Connection.SendMessage(request.Serialize());
		}

		private void IsInputUInt16(object sender, TextCompositionEventArgs e)
		{
			e.Handled = !UInt16.TryParse(OptNetworkPort.Text + e.Text, out _);
		}

		private void OptHostname_TextChanged(object sender, TextChangedEventArgs e)
		{
			Connection.ServerHostname = OptHostname.Text;
		}

		private void OptNetworkPort_TextChanged(object sender, TextChangedEventArgs e)
		{
			if (ushort.TryParse(OptNetworkPort.Text, out ushort val))
				Connection.ServerPort = val;
		}
	}
}
