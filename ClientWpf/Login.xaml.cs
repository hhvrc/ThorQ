using Newtonsoft.Json;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
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

			// Popup
			PopupOkButton.Click += PopupOkButton_Click;

			// Login tab
			LoginButton.Click += LoginButton_Click;
			LoginUsernameInput.TextChanged += LoginInputsValidator;
			LoginPasswordInput.PasswordChanged += LoginInputsValidator;

			// Register tab
			RegisterButton.Click += RegisterButton_Click;
			RegisterUsernameInput.TextChanged += RegisterInputsValidator;
			RegisterEmailInput.TextChanged += RegisterInputsValidator;
			RegisterPasswordInput.PasswordChanged += RegisterInputsValidator;
			RegisterPasswordVerifyInput.PasswordChanged += RegisterInputsValidator;

			// Network region
			NetworkHostInput.TextChanged += NetworkHostInput_Changed;
			NetworkPortInput.TextChanged += NetworkPortInput_Changed;
			NetworkPortInput.PreviewTextInput += NetworkPortInputFormatter;
			NetworkTestButton.Click += NetworkTestButton_Click;

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
			CollarConnectTestButton.Click += CollarConnectTestButton_MouseDown;

			// Connection setup
			NetworkHostInput.Text = Connection.Hostname;
			NetworkPortInput.Text = Connection.Port.ToString();
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
			else
				this.Show();
		}
		#endregion
		#region MessageHandlers
		private void Connection_ServerMessageReceived(CollarLib.Response resp)
		{
			try
			{
				switch (resp.code)
				{
					case CollarLib.ResponseCode.OK:
					case CollarLib.ResponseCode.ACCEPTED:
					case CollarLib.ResponseCode.CREATED:
					case CollarLib.ResponseCode.DELETED:
					case CollarLib.ResponseCode.NOPE:
					case CollarLib.ResponseCode.ERROR:
					case CollarLib.ResponseCode.FORBIDDEN:
					case CollarLib.ResponseCode.UNAUTHORIZED:
					case CollarLib.ResponseCode.INVALID_PARAMS:
					case CollarLib.ResponseCode.INVALID_REQUEST:
						Console.WriteLine($"Got an unrequested response, this should not happen!\n{resp.payload}");
						break;
					case CollarLib.ResponseCode.UPDATE_DATA:
						HandleUpdateData(resp);
						return;
					case CollarLib.ResponseCode.ADMIN_MSG:
						HandleAdminMsg(resp);
						return;
					default:
						Console.WriteLine($"Got an unrecognized response:\n{resp.payload}");
						return;
				}
			}
			catch (Exception ex)
			{
				Console.WriteLine(ex.Message);
				return;
			}
		}
		private void HandleAdminMsg(CollarLib.Response resp)
		{
			if (resp.type == CollarLib.ResponseType.STRING)
			{
				ShowPopup($"SERVER: {resp.payload}");
			}
		}
		private void HandleUpdateData(CollarLib.Response resp)
		{
			switch (resp.type)
			{
				case CollarLib.ResponseType.NULL:
					break;
				case CollarLib.ResponseType.STRING:
					break;
				case CollarLib.ResponseType.RPC:
					break;
				case CollarLib.ResponseType.P2PR:
					break;
				case CollarLib.ResponseType.ACCOUNT:
					break;
				case CollarLib.ResponseType.BLOCKED_USER:
					break;
				case CollarLib.ResponseType.BLOCKED_USER_LIST:
					break;
				case CollarLib.ResponseType.FRIEND:
					break;
				case CollarLib.ResponseType.FRIEND_LIST:
					break;
				case CollarLib.ResponseType.FRIEND_REQUEST:
					break;
				case CollarLib.ResponseType.FRIEND_REQUEST_LIST:
					break;
				case CollarLib.ResponseType.MESSAGE:
					break;
				case CollarLib.ResponseType.MESSAGE_LIST:
					break;
				case CollarLib.ResponseType.CONVERSATION:
					break;
				case CollarLib.ResponseType.CONVERSATION_LIST:
					break;
				default:
					break;
			}
		}
		#endregion
		#region PopupWindow
		private void ShowPopup(String popupMsg)
		{
			DispatcherOperation op = Dispatcher.BeginInvoke((Action)(() =>
			{
				PopupText.Text = popupMsg;
				Popup1.Width = PopupText.Width;
				Popup1.IsOpen = true;
			}));
		}
		private void PopupOkButton_Click(object sender, RoutedEventArgs e)
		{
			this.IsEnabled = true;
			Popup1.IsOpen = false;
		}
		#endregion
		#region LoginWindow
		private void LoginButton_Click(object sender, RoutedEventArgs e)
		{
			if (!Connection.Connect())
			{
				ShowPopup("Cant connect to server!");
				return;
			}

			if (String.IsNullOrWhiteSpace(LoginUsernameInput.Text) || String.IsNullOrWhiteSpace(LoginPasswordInput.Password))
				return;

			CollarLib.ClientPayloads.AccountGetRequest payload = new CollarLib.ClientPayloads.AccountGetRequest()
			{
				username = LoginUsernameInput.Text,
				password = LoginPasswordInput.Password,
			};

			Connection.SendMessage(payload.Serialize(), CollarLib.RequestMethod.GET, CollarLib.RequestType.Account, (resp) =>
			{
				try
				{
					if (resp.code != CollarLib.ResponseCode.OK)
					{
						ShowPopup("INVALID PARAMETERS: " + resp.payload);
						return;
					}

					Console.WriteLine($"Got: {resp.payload}");

					CollarLib.ServerPayloads.AccountInstance account = CollarLib.ServerPayloads.AccountInstance.Deserialize(resp.payload);

					Instance instance = new Instance();
					instance.username = account.Username;
					instance.status = account.Status;
					instance.activity = account.Activity;
					instance.friends = account.Friends;

					foreach (var blockedUser in account.BlockedUsers)
					{
						instance.blockedUsers.Add(
							new CollarLib.BlockedUser(
								blockedUser.BlockId,
								blockedUser.FrozenUsername
								)
							);
					}

					instance.friendRequests = account.FriendRequests;

					foreach (var convo in account.Conversations)
					{
						instance.conversations.Add(
							new CollarLib.Conversation(
								convo.Id,
								convo.Name,
								convo.Members
								)
							);
					}

					DispatcherOperation op = Dispatcher.BeginInvoke((Action)(() =>
					{
						mainWindow.ActiveInstance = instance;
						mainWindow.Show();
					}));
				}
				catch (Exception ex)
				{
					Console.WriteLine(ex.Message);
					return;
				}
			});
		}
		private void LoginInputsValidator(object sender, RoutedEventArgs e)
		{
			if (String.IsNullOrWhiteSpace(LoginUsernameInput.Text))
			{
				LoginButton.IsEnabled = false;
				loginErrorTextbox.Text = "Username cannot be empty!";
				loginErrorTextbox.Visibility = Visibility.Visible;
				return;
			}
			if (String.IsNullOrWhiteSpace(LoginPasswordInput.Password))
			{
				LoginButton.IsEnabled = false;
				loginErrorTextbox.Text = "Password cannot be empty!";
				loginErrorTextbox.Visibility = Visibility.Visible;
				return;
			}
			loginErrorTextbox.Visibility = Visibility.Hidden;
			LoginButton.IsEnabled = true;
		}
		#endregion
		#region RegistrationWindow
		private void RegisterInputsValidator(object sender, RoutedEventArgs e)
		{
			if (String.IsNullOrWhiteSpace(RegisterUsernameInput.Text))
			{
				RegisterButton.IsEnabled = false;
				registerErrorTextbox.Text = "Username cannot be empty!";
				registerErrorTextbox.Visibility = Visibility.Visible;
				return;
			}
			if (String.IsNullOrWhiteSpace(RegisterEmailInput.Text))
			{
				RegisterButton.IsEnabled = false;
				registerErrorTextbox.Text = "Email cannot be empty!";
				registerErrorTextbox.Visibility = Visibility.Visible;
				return;
			}
			if (!HeavenLib.ToolBox.IsValidEmail(RegisterEmailInput.Text))
			{
				RegisterButton.IsEnabled = false;
				registerErrorTextbox.Text = "Invalid email!";
				registerErrorTextbox.Visibility = Visibility.Visible;
				return;
			}
			if (RegisterPasswordInput.Password != RegisterPasswordVerifyInput.Password)
			{
				RegisterButton.IsEnabled = false;
				registerErrorTextbox.Text = "Passwords dont match!";
				registerErrorTextbox.Visibility = Visibility.Visible;
				return;
			}
			if (String.IsNullOrWhiteSpace(RegisterPasswordInput.Password))
			{
				RegisterButton.IsEnabled = false;
				registerErrorTextbox.Text = "Password cannot be empty!";
				registerErrorTextbox.Visibility = Visibility.Visible;
				return;
			}
			registerErrorTextbox.Visibility = Visibility.Hidden;
			RegisterButton.IsEnabled = true;
		}
		private void RegisterButton_Click(object sender, RoutedEventArgs e)
		{
			if (!Connection.Connect())
			{
				ShowPopup("Cant connect to server!");
				return;
			}

			String username = RegisterUsernameInput.Text;
			String email = RegisterEmailInput.Text;
			String password = (RegisterPasswordInput.Password == RegisterPasswordVerifyInput.Password) ? RegisterPasswordInput.Password : null;

			if (String.IsNullOrWhiteSpace(username) || String.IsNullOrWhiteSpace(email) || String.IsNullOrWhiteSpace(password))
				return;

			CollarLib.ClientPayloads.AccountPostRequest payload = new CollarLib.ClientPayloads.AccountPostRequest()
			{
				username = username,
				email = email,
				password = password
			};

			Connection.SendMessage(payload.Serialize(), CollarLib.RequestMethod.POST, CollarLib.RequestType.Account, (resp) =>
			{
				try
				{
					if (resp.code != CollarLib.ResponseCode.OK)
					{
						ShowPopup("INVALID PARAMETERS: " + resp.payload);
						return;
					}

					Console.WriteLine($"Got: {resp.payload}");

					CollarLib.ServerPayloads.AccountInstance account = CollarLib.ServerPayloads.AccountInstance.Deserialize(resp.payload);

					Instance instance = new Instance();
					instance.username = account.Username;
					instance.status = account.Status;
					instance.activity = account.Activity;
					instance.friends = account.Friends;

					foreach (var blockedUser in account.BlockedUsers)
					{
						instance.blockedUsers.Add(
							new CollarLib.BlockedUser(
								blockedUser.BlockId,
								blockedUser.FrozenUsername
								)
							);
					}

					instance.friendRequests = account.FriendRequests;

					foreach (var convo in account.Conversations)
					{
						instance.conversations.Add(
							new CollarLib.Conversation(
								convo.Id,
								convo.Name,
								convo.Members
								)
							);
					}

					mainWindow.ActiveInstance = instance;
				}
				catch (Exception ex)
				{
					Console.WriteLine(ex.Message);
					return;
				}
			});
		}
		#endregion
		#region NetworkRegion
		private void NetworkPortInputFormatter(object sender, TextCompositionEventArgs e)
		{
			e.Handled = !UInt16.TryParse(NetworkPortInput.Text + e.Text, out _);
		}
		private void NetworkHostInput_Changed(object sender, TextChangedEventArgs e)
		{
			Connection.Hostname = NetworkHostInput.Text;
		}
		private void NetworkPortInput_Changed(object sender, TextChangedEventArgs e)
		{
			if (ushort.TryParse(NetworkPortInput.Text, out ushort val))
				Connection.Port = val;
		}
		private void NetworkTestButton_Click(object sender, RoutedEventArgs e)
		{
			PopupText.Text = Connection.TestAddress(NetworkHostInput.Text, UInt16.Parse(NetworkPortInput.Text)) ? "Looks ok!" : "Wrong hostname/port";
			this.IsEnabled = false;
			Popup1.IsOpen = true;
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
		private void CollarConnectTestButton_MouseDown(object sender, RoutedEventArgs e)
		{
			SerialConnection.Open();
			SerialConnection.SendCommand('+');
			Thread.Sleep(500);
			SerialConnection.SendCommand('#');
			SerialConnection.SendCommand('#');
			SerialConnection.SendCommand('#');
			SerialConnection.Close();
		}
		#endregion
	}
}
