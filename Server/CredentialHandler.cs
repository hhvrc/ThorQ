using System;
using System.IO;
using System.Security;
using System.Security.Cryptography;
using System.Threading;
using BCrypt.Net;
using System.Text;

namespace CollarControl
{
    public class CredentialHandler
    {
        private readonly int cost = 16; //4-31 (power of two iterations)(at 31 your cpu will want to die) 10 is default
        private string exePath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().CodeBase);
        private string credPath = "";

        [Serializable]
        public struct Creds
        {
            public bool isValid;
            public string mailAddress;
            public string username;
            public string passwordHash;
        }

        private static string EncodeTo64(string toEncode)
        {
            byte[] toEncodeAsBytes = System.Text.ASCIIEncoding.ASCII.GetBytes(toEncode);
            string returnValue = System.Convert.ToBase64String(toEncodeAsBytes);

            return returnValue;
        }
        private static string DecodeFrom64(string encodedData)
        {
            byte[] encodedDataAsBytes = System.Convert.FromBase64String(encodedData);
            string returnValue = System.Text.ASCIIEncoding.ASCII.GetString(encodedDataAsBytes);

            return returnValue;
        }

        public CredentialHandler()
        {
            exePath = exePath.Remove(0, 6);
            credPath = exePath + "\\creds.bin";
            if (!File.Exists(credPath))
            {
                File.Create(credPath);
            }
        }

        public void SetCredentials(string userName, string mailAddress, string password)
        {
            if (userName == null || mailAddress == null || password == null) { return; }

            string passwordHash = BCrypt.Net.BCrypt.EnhancedHashPassword(mailAddress + password, BCrypt.Net.HashType.SHA512, cost);
            Console.WriteLine(passwordHash);
            Console.WriteLine("Hash match: " + BCrypt.Net.BCrypt.EnhancedVerify(mailAddress + password, passwordHash, BCrypt.Net.HashType.SHA512));

            string filecontent = EncodeTo64(userName) + '#' + EncodeTo64(mailAddress) + '#' + EncodeTo64(passwordHash);

            MD5 md5 = MD5.Create();
            byte[] md5Hash = md5.ComputeHash(Encoding.UTF8.GetBytes(filecontent));
            filecontent += '-' + EncodeTo64(Encoding.UTF8.GetString(md5Hash, 0, md5Hash.Length));

            Console.WriteLine(filecontent);

            File.WriteAllText(credPath, filecontent);
        }
        public Creds GetCredentials()
        {
            string fileContent = File.ReadAllText(credPath);

            string[] fileData = fileContent.Split('-');
            if (fileData.Length != 2)
            {
                return new Creds() { isValid = false };
            }
            string[] elements = fileContent.Split('#');
            if (elements.Length != 4)
            {
                return new Creds() { isValid = false };
            }


            MD5 md5 = MD5.Create();
            //Console.WriteLine(hashes[1]);
            byte[] md5Hash = md5.ComputeHash(Encoding.UTF8.GetBytes(fileData[1]));
            Console.WriteLine("File content intact: " + EncodeTo64(Encoding.UTF8.GetString(md5Hash, 0, md5Hash.Length)) == fileData[1]);


            Creds creds = new Creds();

            creds.isValid = true;
            creds.username = DecodeFrom64(elements[0]);
            creds.mailAddress = DecodeFrom64(elements[1]);
            creds.passwordHash = DecodeFrom64(elements[2].Split('#')[0]);

            return creds;
        }
    }
}
