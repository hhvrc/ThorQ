using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Web;

namespace ThorQ
{
	public static class Crypto
	{
		private static readonly string salt = "3499bd that form the signature that is created adhSSHFDLn";

		public static string SignMessage(string message, string username, string privateKey)
		{
			return EncryptWithSha256(message + " Key:" + GetPublicKey(username, privateKey));
		}

		public static string SignMessage(string message, string publicKey)
		{
			return EncryptWithSha256(message + " Key:" + publicKey);
		}

		public static string GetPublicKey(string username, string privateKey)
		{
			return EncryptWithSha256(username.ToLower() + "_key:" + privateKey + salt);
		}

		public static bool IsValid(string message, string publicKey, string signature)
		{
			return EncryptWithSha256(message + " Key:" + publicKey) == signature;
		}

		public static string EncryptWithSha256(string input)
		{
			string signature = "";

			using (SHA256 sha256 = SHA256.Create())
			{
				byte[] stringBytes = Encoding.UTF8.GetBytes(input.ToCharArray());

				byte[] hashValue = sha256.ComputeHash(stringBytes);

				signature = ToHex(hashValue, true);
			}

			if (!String.IsNullOrEmpty(signature))
			{
				return signature;
			}
			else
			{
				return null;
			}
		}

		private static string ToHex(byte[] bytes, bool upperCase)
		{
			StringBuilder result = new StringBuilder(bytes.Length * 2);

			for (int i = 0; i < bytes.Length; i++)
			{
				result.Append(bytes[i].ToString(upperCase ? "X2" : "x2"));
			}

			return result.ToString();
		}
	}
}