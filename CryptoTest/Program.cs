using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using HeavenLib.Security;

namespace CryptoTest
{
	class Program
	{
		static void Main(string[] args)
		{
			var threadStart = new ThreadStart(() =>
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
			});

			var thr1 = new Thread(threadStart);
			var thr2 = new Thread(threadStart);
			var thr3 = new Thread(threadStart);
			var thr4 = new Thread(threadStart);
			var thr5 = new Thread(threadStart);
			var thr6 = new Thread(threadStart);
			thr1.Start();
			thr2.Start();
			thr3.Start();
			thr4.Start();
			thr5.Start();
			thr6.Start();
			thr1.Join();
			thr2.Join();
			thr3.Join();
			thr4.Join();
			thr5.Join();
			thr6.Join();
		}
	}
}
