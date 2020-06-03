using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThorQ
{
    static class SerialConnection
	{
		private static object l_port = new object();
		private static SerialPort m_port = null;

		public static bool IsOpen
		{
			get
			{
				lock (l_port)
					return m_port?.IsOpen ?? false;
			}
		}
		public static String PortName
		{
			get
			{
				if (HeavenLib.AppConfig.TryGet("CollarPortName", out String str))
					return str;
				return null;
			}
			set
			{
				HeavenLib.AppConfig.Upsert("CollarPortName", ((value == null) ? "" : value));
			}
		}
		public static int BaudRate
		{
			get
			{
				if (HeavenLib.AppConfig.TryGet("CollarBaudRate", out String str))
					if (int.TryParse(str, out int port))
						return port;
				return 0;
			}
			set
			{
				HeavenLib.AppConfig.Upsert("CollarBaudRate", value.ToString());
			}
		}
		public static bool Open()
		{
			lock (l_port)
			{
				if (m_port != null)
				{
					try { m_port?.Dispose(); } catch (Exception) { }
					m_port = null;
				}
				try
				{
					m_port = new SerialPort(PortName, BaudRate);
					m_port.Open();
					if (m_port.IsOpen)
						return true;
				}
				catch (Exception) { }
				try { m_port?.Dispose(); } catch (Exception) { }
				m_port = null;
				return false;
			}
		}
		public static void Close()
		{
			lock (l_port)
			{
				try { m_port?.Close(); } catch (Exception) { }
				m_port = null;
			}
		}
		public static void SendCommand(char cmd)
		{
			lock (l_port)
			{
				m_port?.Write(new byte[] {(byte)cmd},0,1);
			}
		}
		public static bool TestPort(string portName, int baudRate)
		{
			bool result;
			try
			{
				var client = new SerialPort(portName, baudRate);
				client.Open();
				result = client.IsOpen;
				client.Close();
			}
			catch (Exception)
			{
				result = false;
			}
			return result;
		}
	}
}
