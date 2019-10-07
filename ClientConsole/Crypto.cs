using System;
using System.IO;
using System.Security.Cryptography;

namespace CollarControl
{
	/// <summary>
	/// Implements AES encryption/decryption and ECDH key-exchange
	/// </summary>
	public class Crypto
	{
		private bool _ready = false;
		private byte[] _publicKey = null;
		private byte[] _privateKey = null;
		private ECDiffieHellmanCng _keyPair = null;

		/// <summary>
		/// Sets up class, and generates public key
		/// </summary>
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

		/// <summary>
		/// Generates shared private key from another public key
		/// </summary>
		/// <param name="key">
		/// Other public key
		/// </param>
		/// <exception cref="ArgumentNullException"></exception>
		/// <exception cref="InvalidOperationException"></exception>
		/// <exception cref="ArgumentException"></exception>
		/// <exception cref="CryptographicException"></exception>
		public void GenPrivateKey(byte[] key)
		{
			if (key == null)
			{
				throw new ArgumentNullException();
			}

			try // DEBUG
			{
				_privateKey = _keyPair.DeriveKeyMaterial(CngKey.Import(key, CngKeyBlobFormat.EccPublicBlob));
			}
			catch (PlatformNotSupportedException ex) // DEBUG
			{
				Console.WriteLine("Well shit... Platform not supported: {0}", ex.Message);
				return;
			}

			_ready = true;
		}

		/// <summary>
		/// Returns public key that was generated at creation of crypto class
		/// </summary>
		/// <returns>
		/// Public key
		/// </returns>
		public byte[] GetPublicKey()
		{
			return _publicKey;
		}

		/// <summary>
		/// Encrypts data
		/// </summary>
		/// <param name="unencryptedData">
		/// Unencrypted bytearray
		/// </param>
		/// <param name="iv">
		/// Outputs the Initial Vector from the encryption of the data
		/// </param>
		/// <returns>
		/// Encrypted data<para/>
		/// Returns null if input is invalid / ECDH-exchange has not occured
		/// </returns>
		public byte[] Encrypt(byte[] unencryptedData, out byte[] iv)
		{
			if (!_ready || unencryptedData == null)
			{
				iv = null;
				return null;
			}

			try // DEBUG
			{
				using (Aes aes = new AesCryptoServiceProvider())
				{
					aes.Key = _privateKey;
					iv = aes.IV;

					// Encrypt the data
					using (MemoryStream encryptedData = new MemoryStream())
					{
						using (CryptoStream stream = new CryptoStream(encryptedData, aes.CreateEncryptor(), CryptoStreamMode.Write))
						{
							stream.Write(unencryptedData, 0, unencryptedData.Length);
							stream.Close();

							return encryptedData.ToArray();
						}
					}
				}
			}
			catch (PlatformNotSupportedException ex) // DEBUG
			{
				Console.WriteLine("Well shit... Platform not supported: {0}", ex.Message);
				iv = null;
				return null;
			}
		}

		/// <summary>
		/// Decrypts encrypted data
		/// </summary>
		/// <param name="encryptedData">
		/// Encrypted array of bytes
		/// </param>
		/// <param name="iv">
		/// Initial Vector Output from encryption of message
		/// </param>
		/// <returns>
		/// Returns the unencrypted data, or <c>null</c> if (input is invalid / ECDH-exchange has not occured)
		public byte[] Decrypt(byte[] encryptedData, byte[] iv)
		{
			if (!_ready || encryptedData == null || iv == null)
			{
				return null;
			}

			try // DEBUG
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
			catch (PlatformNotSupportedException ex) // DEBUG
			{
				Console.WriteLine("Well shit... Platform not supported: {0}", ex.Message);
				return null;
			}
		}
	}
}
