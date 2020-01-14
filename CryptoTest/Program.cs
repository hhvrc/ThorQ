using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using CollarControl;

namespace CryptoTest
{
	class Program
	{
		static void Main(string[] args)
		{
			for (int x = 0; x < 100; x++)
			{
				Crypto c1 = new Crypto();
				Crypto c2 = new Crypto();

				c2.EstablishSecretKey(c1.GetPublicKey());
				c1.EstablishSecretKey(c2.GetPublicKey());

				Console.WriteLine(Encoding.UTF8.GetString(c2.Decrypt(c1.Encrypt(Encoding.UTF8.GetBytes("This is a test")))));

				for (int y = 0; y < 100; y++)
				{
					Console.WriteLine(Encoding.UTF8.GetString(c2.Decrypt(c1.Encrypt(Encoding.UTF8.GetBytes("This is a test")))));
				}
			}
		}
	}
}
