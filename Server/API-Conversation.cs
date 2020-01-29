using LiteDB;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CollarControl
{
	class ConversationAPI
	{
		private LiteCollection<Conversation> _dbConversations;

		public ConversationAPI(LiteDatabase db)
		{
			_dbConversations = db.GetCollection<Conversation>("conversations");
		}

		/// <summary>
		/// Adds a new conversation
		/// </summary>
		/// <param name="members"></param>
		/// <returns>
		/// Id of conversation
		/// </returns>
		public Guid Add(String name, List<Guid> members)
		{
			Conversation convo = new Conversation();
			convo.name = name;
			convo.members = members;
			_dbConversations.Insert(convo);
			return convo.id;
		}

		public String GetName(Guid conversationId)
		{
			Conversation convo = _dbConversations.FindById(conversationId);
			if (convo == null)
				return null;
			return convo.name;
		}
		public bool SetName(Guid conversationId, String newName)
		{
			Conversation convo = _dbConversations.FindById(conversationId);
			if (convo == null)
				return false;
			convo.name = newName;
			_dbConversations.Update(convo);
			return true;
		}

		public bool MemberAdd(Guid conversationId, Guid memberId)
		{
			Conversation convo = _dbConversations.FindById(conversationId);

			if (convo == null || convo.members.Contains(memberId))
				return false;

			convo.members.Add(memberId);

			_dbConversations.Update(convo);

			return true;
		}
		public bool MemberRemove(Guid conversationId, Guid memberId)
		{
			Conversation convo = _dbConversations.FindById(conversationId);

			if (convo == null || !convo.members.Contains(memberId))
				return false;

			convo.members.Remove(memberId);

			if (convo.members.Count == 0)
				_dbConversations.Delete(c => c.id == convo.id);
			else
				_dbConversations.Update(convo);
			return true;
		}
		public List<Guid> MemberList(Guid conversationId)
		{
			Conversation convo = _dbConversations.FindById(conversationId);
			if (convo == null)
				return null;
			return convo.members;
		}

		public Message? AddMessage(Guid conversationId, Guid memberFrom, String content)
		{
			Conversation convo = _dbConversations.FindById(conversationId);
			if (convo == null || !convo.members.Contains(memberFrom))
				return null;

			Message msg = new Message()
			{
				id = Guid.NewGuid(),
				usrId = memberFrom,
				utcTime = DateTime.UtcNow,
				content = content,
			};

			convo.messages.Add(msg);

			_dbConversations.Update(convo);

			return msg;
		}
		public List<Message> GetMessages(Guid conversationId, ulong offset, ulong nMessages)
		{
			Conversation convo = _dbConversations.FindById(conversationId);
			if (convo == null || convo.messages.Count < 0)
				return null;

			if (offset > (ulong)convo.messages.Count)
				return null;

			nMessages = Math.Min((uint)convo.messages.Count - offset, nMessages);

			return convo.messages.GetRange((int)offset, (int)nMessages);
		}
	}
}
