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
		private byte[] _sharedKey = null;
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
				_keyPair.Dispose();
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
		public void EstablishSecretKey(byte[] key)
		{
			_ready = false;
			if (key == null)
			{
				throw new ArgumentNullException();
			}

			try // DEBUG
			{
				_sharedKey = _keyPair.DeriveKeyMaterial(CngKey.Import(key, CngKeyBlobFormat.EccPublicBlob));
			}
			catch (Exception ex) // DEBUG
			{
				Console.WriteLine("Exception caught: {0}", ex.Message);
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
		public byte[] Encrypt(byte[] unencryptedData)
		{
			if (!_ready || unencryptedData == null || unencryptedData.Length == 0)
				return null;

			try // DEBUG
			{
				using (Aes aes = new AesCryptoServiceProvider())
				{
					aes.Key = _sharedKey;
					byte[] iv = aes.IV;

					// Encrypt the data
					using (MemoryStream ms = new MemoryStream())
					{
						using (CryptoStream cs = new CryptoStream(ms, aes.CreateEncryptor(), CryptoStreamMode.Write))
							cs.Write(unencryptedData, 0, unencryptedData.Length);

						byte[] encryptedContent = ms.ToArray();

						byte[] result = new byte[iv.Length + encryptedContent.Length];

						//copy our 2 array into one
						System.Buffer.BlockCopy(iv, 0, result, 0, iv.Length);
						System.Buffer.BlockCopy(encryptedContent, 0, result, iv.Length, encryptedContent.Length);

						return result;
					}
				}
			}
			catch (Exception ex) // DEBUG
			{
				Console.WriteLine("Exception caught: {0}", ex.Message);
			}
			return null;
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
		/// </returns>
		public byte[] Decrypt(byte[] encryptedData)
		{
			if (!_ready || encryptedData == null || encryptedData.Length <= 16)
				return null;

			byte[] iv = new byte[16];
			byte[] dat = new byte[encryptedData.Length - iv.Length];

			System.Buffer.BlockCopy(encryptedData, 0, iv, 0, iv.Length);
			System.Buffer.BlockCopy(encryptedData, iv.Length, dat, 0, dat.Length);

			try // DEBUG
			{
				using (Aes aes = new AesCryptoServiceProvider())
				{
					aes.Key = _sharedKey;
					aes.IV = iv;

					// Decrypt the data
					using (MemoryStream ms = new MemoryStream())
					{
						using (CryptoStream cs = new CryptoStream(ms, aes.CreateDecryptor(), CryptoStreamMode.Write))
							cs.Write(dat, 0, dat.Length);

						return ms.ToArray();
					}
				}
			}
			catch (Exception ex) // DEBUG
			{
				Console.WriteLine("Exception caught: {0}", ex.Message);
			}
			return null;
		}
	}
}