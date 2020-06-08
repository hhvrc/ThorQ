using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace ThorQ
{
	/// <summary>
	/// Interaction logic for Window1.xaml
	/// </summary>
	public partial class MainWindow : Window
	{
		public MainWindow()
		{
			InitializeComponent();
		}

		private object l_instance = new object();
		private Instance m_instance;
		public Instance ActiveInstance
		{
			get
			{
				lock (l_instance)
					return m_instance;
			}
			set
			{
				lock (l_instance)
					m_instance = value;
			}
		}

		private void OnLogoutButtonClicked(object sender, RoutedEventArgs e)
		{
			this.Hide();
		}
	}
}
