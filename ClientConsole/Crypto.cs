using System;
using System.IO;
using System.Security.Cryptography;

namespace CollarControl
{
	public class Crypto
	{
		private bool _ready = false;
		private byte[] _publicKey = null;
		private byte[] _privateKey = null;
		private ECDiffieHellmanCng _keyPair = null;

		public Crypto()
		{
			_keyPair = new ECDiffieHellmanCng
			{
				KeyDerivationFunction = ECDiffieHellmanKeyDerivationFunction.Hash,
				HashAlgorithm = CngAlgorithm.Sha256
			};
			_publicKey = _keyPair.PublicKey.ToByteArray();
		}

		~Crypto()
		{
			if (_keyPair != null)
			{
				_keyPair.Dispose();
				_keyPair = null;
			}
			_publicKey = null;
			_privateKey = null;
		}

		public void GenPrivateKey(byte[] key)
		{
			try
			{
				_privateKey = _keyPair.DeriveKeyMaterial(CngKey.Import(key, CngKeyBlobFormat.EccPublicBlob));
			}
			catch (Exception ex)
			{
				throw new Exception("Couldn't generate private key: " + ex.Message);
			}
			_ready = true;
		}

		public byte[] GetPublicKey()
		{
			return _publicKey;
		}
		public byte[] Encrypt(byte[] unencryptedData, out byte[] iv)
		{
			if (unencryptedData == null)
			{
				iv = null;
				return null;
			}
			if (!_ready)
			{
				iv = null;
				return null;
			}

			try
			{
				using (Aes aes = new AesCryptoServiceProvider())
				{
					aes.Key = _privateKey;
					iv = aes.IV;

					// Encrypt the data
					using (MemoryStream encryptedData = new MemoryStream())
					using (CryptoStream stream = new CryptoStream(encryptedData, aes.CreateEncryptor(), CryptoStreamMode.Write))
					{
						stream.Write(unencryptedData, 0, unencryptedData.Length);
						stream.Close();

						return encryptedData.ToArray();
					}
				}
			}
			catch (Exception ex)
			{
				throw new Exception("Couldn't decrypt data: " + ex.Message);
			}
		}
		public byte[] Decrypt(byte[] encryptedData, byte[] iv)
		{
			if (!_ready || encryptedData == null || iv == null)
			{
				return null;
			}

			try
			{
				using (Aes aes = new AesCryptoServiceProvider())
				{
					aes.Key = _privateKey;
					aes.IV = iv;

					// Decrypt the data
					using (MemoryStream decryptedData = new MemoryStream())
					{
						using (CryptoStream stream = new CryptoStream(decryptedData, aes.CreateDecryptor(), CryptoStreamMode.Write))
						{
							stream.Write(encryptedData, 0, encryptedData.Length);
							stream.Close();

							return decryptedData.ToArray();
						}
					}
				}
			}
			catch (Exception ex)
			{
				throw new Exception("Couldn't decrypt data: " + ex.Message);
			}
		}
	}
}
