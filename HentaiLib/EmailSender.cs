using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Mail;
using System.Text;
using System.Threading.Tasks;
using System.Net;

namespace HeavenLib
{
    public class EmailClient
	{
		SmtpClient smtpClient;
		MailAddress mailAddress;
		NetworkCredential credential;

		public EmailClient(String host, int port, String email, String password)
		{
			smtpClient = new SmtpClient(host);
			mailAddress = new MailAddress(email);
			credential = new NetworkCredential(email, password);

			smtpClient.Port = port;
			smtpClient.Credentials = credential;
			smtpClient.EnableSsl = true;
		}

		public bool SendEmail(string[] recepients, string subject, string body)
		{
			if (recepients == null || string.IsNullOrWhiteSpace(subject) || string.IsNullOrWhiteSpace(subject))
				return false;

			MailMessage mail = new MailMessage();

			mail.From = mailAddress;
			foreach (string recepient in recepients)
				if (ToolBox.IsValidEmail(recepient))
					mail.Bcc.Add(recepient);
			mail.Subject = subject;
			mail.Body = body;

			smtpClient.Send(mail);
			return true;
		}
	}
}
