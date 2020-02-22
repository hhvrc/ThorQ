using System;
using System.IO;
using System.Security.Cryptography;
using System.Text;
using System.Xml.Serialization;

namespace HeavenLib
{
	public static class ToolBox
	{
		private const String charList = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/-_=?!#&%()@${[]}*.:,;<>";

		public static T Deserialize<T>(byte[] xmlData)
		{
			if (xmlData == null)
				return default;

			var stringReader = new System.IO.StringReader(Encoding.UTF8.GetString(xmlData));
			var serializer = new XmlSerializer(typeof(T));
			return (T)serializer.Deserialize(stringReader);
		}

		public static byte[] Serialize<T>(T dataToSerialize)
		{
			if (dataToSerialize == null)
				return default;

			var stringwriter = new System.IO.StringWriter();
			var serializer = new XmlSerializer(typeof(T));
			serializer.Serialize(stringwriter, dataToSerialize);
			return Encoding.UTF8.GetBytes(stringwriter.ToString());
		}

		public static T[] CompileArrays<T>(T[] array, T[] appendArray, params T[][] additional)
		{
			long resultLength = array.Length + appendArray.Length;
			long offsetSize = resultLength;

			foreach (T[] arr in additional)
			{
				resultLength += arr.Length;
			}

			T[] result = new T[resultLength];

			array.CopyTo(result, 0);
			appendArray.CopyTo(result, array.Length);

			foreach (T[] arr in additional)
			{
				arr.CopyTo(result, offsetSize);
				offsetSize += arr.Length;
			}

			return result;
		}

		public static T[] SubArray<T>(this T[] data, int index, int length)
		{
			T[] result = new T[length];
			Array.Copy(data, index, result, 0, length);
			return result;
		}

		public static byte[] ToNetworkLayer(int value)
		{
			byte[] bytes = BitConverter.GetBytes(value);
			if (BitConverter.IsLittleEndian)
				Array.Reverse(bytes);
			return bytes;
		}

		public static int ToHostLayer(byte[] bytes)
		{
			if (BitConverter.IsLittleEndian)
				Array.Reverse(bytes);
			return BitConverter.ToInt32(bytes, 0);
		}

		/// <summary>
		/// Returns if the input string is a valid email format
		/// </summary>
		/// <param name="email">The email to be evaluated</param>
		/// <returns>
		/// Is format valid?
		/// </returns>
		public static bool IsValidEmail(string email)
		{
			if (!String.IsNullOrWhiteSpace(email))
			{
				try
				{
					var addr = new System.Net.Mail.MailAddress(email);
					return addr.Address == email;
				}
				catch (FormatException) { }
			}
			return false;
		}

		/// <summary>
		/// Generates a unique token of specified length using cryptographic functions
		/// </summary>
		/// <param name="length">
		/// Length of output token
		/// </param>
		/// <returns>
		/// The generated token
		/// </returns>
		/// <exception cref="CryptographicException"></exception>
		public static string GetUniqueToken(int length)
		{
			using (RNGCryptoServiceProvider crypto = new RNGCryptoServiceProvider())
			{
				byte[] data = new byte[length];

				// If chars.Length isn't a power of 2 then there is a bias if we simply use the modulus operator. The first characters of chars will be more probable than the last ones.
				// buffer used if we encounter an unusable random byte. We will regenerate it in this buffer
				byte[] buffer = null;

				// Maximum random number that can be used without introducing a bias
				int maxRandom = byte.MaxValue - ((byte.MaxValue + 1) % charList.Length);

				crypto.GetBytes(data);

				char[] result = new char[length];

				for (int i = 0; i < length; i++)
				{
					byte value = data[i];

					while (value > maxRandom)
					{
						if (buffer == null)
						{
							buffer = new byte[1];
						}

						crypto.GetBytes(buffer);
						value = buffer[0];
					}

					result[i] = charList[value % charList.Length];
				}

				return new string(result);
			}
		}

		/// <summary>
		/// Gets folder containing the compiled executable
		/// </summary>
		/// <returns>
		/// The path to the executable
		/// </returns>
		public static string GetExeDirectory()
		{
			string path = Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().CodeBase);
			if (path.Substring(0, 5) == "file:")
				path = path.Substring(6);
			return path;
		}
	}
}
