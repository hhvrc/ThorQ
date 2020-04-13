using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace HeavenLib.Connectivity
{
    public static class Utils
    {
        public static bool TryGetIPAddress(String hostname, out IPAddress ip)
		{
			ip = IPAddress.None;
			IPAddress[] addresses;
			try
			{
				addresses = Dns.GetHostEntry(hostname).AddressList;
			}
			catch (ArgumentOutOfRangeException)
			{
				Console.WriteLine("hostUri is more than 255 characters long.");
				return false;
			}
			catch (SocketException ex)
			{
				Console.WriteLine("An error was encountered when resolving the hostUri: {0}", ex.Message);
				return false;
			}
			catch (ArgumentException)
			{
				Console.WriteLine("hostUri is an invalid IP address");
				return false;
			}

			if (addresses.Length == 0 || addresses[0] == null)
			{
				Console.WriteLine("Not found");
				return false;
			}

			ip = addresses[0];
			return true;
		}
    }
}
