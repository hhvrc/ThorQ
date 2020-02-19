using Newtonsoft.Json;
using System;
using System.Collections.Concurrent;
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
			UsernameInput.TextChanged += ValidateLoginInputText;
			PasswordInput.PasswordChanged += ValidateLoginInputPassw;
			CollarConnectCheckbutton.Click += CollarConnectCheckbutton_Click;
			ConnectionTestButton.Click += ConnectionTestButton_Click;
			PopupOkButton.Click += PopupOkButton_Click;

			Connection.ServerMessageReceived += OnNonRequestedResponse;
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

		private void OnNonRequestedResponse(CollarLib.Response resp)
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

		private void ValidateLoginInputPassw(object sender, RoutedEventArgs e)
		{
			if (String.IsNullOrWhiteSpace(UsernameInput.Text))
			{
				LoginButton.IsEnabled = false;
				loginErrorTextbox.Text = "Username cannot be empty!";
				loginErrorTextbox.Visibility = Visibility.Visible;
				return;
			}

			if (String.IsNullOrWhiteSpace(PasswordInput.Password))
			{
				LoginButton.IsEnabled = false;
				loginErrorTextbox.Text = "Password cannot be empty!";
				loginErrorTextbox.Visibility = Visibility.Visible;
				return;
			}

			loginErrorTextbox.Visibility = Visibility.Hidden;
			LoginButton.IsEnabled = true;
		}

		private void ValidateLoginInputText(object sender, TextChangedEventArgs e)
		{
			ValidateLoginInputPassw(sender, e);
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

			if (String.IsNullOrWhiteSpace(UsernameInput.Text) || String.IsNullOrWhiteSpace(PasswordInput.Password))
				return;

			CollarLib.ClientPayloads.AccountGetRequest payload = new CollarLib.ClientPayloads.AccountGetRequest()
			{
				username = UsernameInput.Text,
				password = PasswordInput.Password,
			};

			Connection.SendMessage(payload.Serialize(), CollarLib.RequestMethod.GET, CollarLib.RequestType.Account, (resp)=>
			{
				try
				{
					if (resp.code != CollarLib.ResponseCode.OK) {
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

					foreach (var blockedUser in account.BlockedUsers) {
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

		private void RegisterButton_Click(object sender, RoutedEventArgs e)
		{
			if (!Connection.Connect())
			{
				ShowPopup("Cant connect to server!");
				return;
			}

			String username = RegUsernameInput.Text;
			String email = RegEmailInput.Text;
			String password = (RegPasswordInput.Password == RegPasswordVerifyInput.Password) ? RegPasswordInput.Password : null;

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

		private void ValidateRegFormInputsPassw(object sender, RoutedEventArgs e)
		{
			if (String.IsNullOrWhiteSpace(RegUsernameInput.Text))
			{
				RegisterButton.IsEnabled = false;
				registerErrorTextbox.Text = "Username cannot be empty!";
				registerErrorTextbox.Visibility = Visibility.Visible;
				return;
			}

			if (String.IsNullOrWhiteSpace(RegEmailInput.Text))
			{
				RegisterButton.IsEnabled = false;
				registerErrorTextbox.Text = "Email cannot be empty!";
				registerErrorTextbox.Visibility = Visibility.Visible;
				return;
			}

			if (!HeavenLib.ToolBox.IsValidEmail(RegEmailInput.Text))
			{
				RegisterButton.IsEnabled = false;
				registerErrorTextbox.Text = "Invalid email!";
				registerErrorTextbox.Visibility = Visibility.Visible;
				return;
			}

			if (RegPasswordInput.Password != RegPasswordVerifyInput.Password)
			{
				RegisterButton.IsEnabled = false;
				registerErrorTextbox.Text = "Passwords dont match!";
				registerErrorTextbox.Visibility = Visibility.Visible;
				return;
			}

			if (String.IsNullOrWhiteSpace(RegPasswordInput.Password))
			{
				RegisterButton.IsEnabled = false;
				registerErrorTextbox.Text = "Password cannot be empty!";
				registerErrorTextbox.Visibility = Visibility.Visible;
				return;
			}

			registerErrorTextbox.Visibility = Visibility.Hidden;
			RegisterButton.IsEnabled = true;
		}

		private void ValidateRegFormInputsText(object sender, TextChangedEventArgs e)
		{
			ValidateRegFormInputsPassw(sender, e);
		}
	}
}
