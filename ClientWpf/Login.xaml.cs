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

			connectionOk = Connection.Connect();
			OptHostname.Text = Connection.ServerHostname;
			OptNetworkPort.Text = Connection.ServerPort.ToString();
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
				loginErrorTextbox.Text = "Cant connect to server!";
				loginErrorTextbox.Visibility = Visibility.Visible;
				return;
			}

			// @TODO
			//Connection.SendMessage()

			mainWindow.Show();
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
