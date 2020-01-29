using Newtonsoft.Json;
using System;

namespace CollarControl.ClientPayloads
{
	[Serializable]
	public struct AccountGetRequest
	{
		public string username;
		public string password;

		public string Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static AccountGetRequest Deserialize(string str)
		{
			return JsonConvert.DeserializeObject<AccountGetRequest>(str);
		}
	}
	[Serializable]
	public struct AccountPostRequest
	{
		public string username;
		public string email;
		public string password;

		public string Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static AccountPostRequest Deserialize(string str)
		{
			return JsonConvert.DeserializeObject<AccountPostRequest>(str);
		}
	}
	[Serializable]
	public struct RecoveryPostRequest
	{
		public string email;
		public string token;
		public string newPassword;

		public String Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static RecoveryPostRequest Deserialize(String str)
		{
			return JsonConvert.DeserializeObject<RecoveryPostRequest>(str);
		}
	}
	[Serializable]
	public struct EmailSetRequest
	{
		public string newEmail;
		public string password;

		public string Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static EmailSetRequest Deserialize(string str)
		{
			return JsonConvert.DeserializeObject<EmailSetRequest>(str);
		}
	}
	[Serializable]
	public struct PasswordSetRequest
	{
		public string oldPassword;
		public string newPassword;

		public string Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static PasswordSetRequest Deserialize(string str)
		{
			return JsonConvert.DeserializeObject<PasswordSetRequest>(str);
		}
	}
	[Serializable]
	public struct FriendDeleteRequest
	{
		Guid messageId;

		public string Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static PasswordSetRequest Deserialize(string str)
		{
			return JsonConvert.DeserializeObject<PasswordSetRequest>(str);
		}
	}
	[Serializable]
	public struct MessageSetRequest
	{
		Guid conversationId;
		Guid messageId;
		String content;
	}
	[Serializable]
	public struct MessagePostRequest
	{
		public Guid conversationId;
		public String content;

		public string Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static MessagePostRequest Deserialize(string str)
		{
			return JsonConvert.DeserializeObject<MessagePostRequest>(str);
		}
	}
	[Serializable]
	public struct MessageDeleteRequest
	{
		public Guid conversationId;
		public Guid messageId;

		public string Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static MessageDeleteRequest Deserialize(string str)
		{
			return JsonConvert.DeserializeObject<MessageDeleteRequest>(str);
		}
	}
	[Serializable]
	public struct ConversationGetRequest
	{
		public Guid conversationId;
		public UInt64 offset;
		public UInt64 nMessages;

		public string Serialize()
		{
			return JsonConvert.SerializeObject(this);
		}
		public static ConversationGetRequest Deserialize(string str)
		{
			return JsonConvert.DeserializeObject<ConversationGetRequest>(str);
		}
	}
}
