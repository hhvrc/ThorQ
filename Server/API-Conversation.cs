using CollarLib;
using LiteDB;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace ThorQ
{
	class ConversationAPI
	{
		private DbCollectionHandler<DbConversation> _dbCollectionHandler;

		public ConversationAPI(LiteDatabase db)
		{
			_dbCollectionHandler = new DbCollectionHandler<DbConversation>(db, "conversations");
		}

		/// <summary>
		/// Adds a new conversation
		/// </summary>
		/// <param name="name"></param>
		/// <param name="members"></param>
		/// <param name="onDone"></param>
		public void Add(string name, List<Guid> members, Action<Guid> onDone)
		{
			var convo = new DbConversation();
			convo.Name = name;
			convo.Members = members;

			_dbCollectionHandler.AddJob((ILiteCollection<DbConversation> dbCollection) =>
			{
				dbCollection.Insert(convo);
				Task.Run(() => onDone.Invoke(convo.Id));
			});
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="conversationId"></param>
		/// <param name="onSuccess"></param>
		/// <param name="onError"></param>
		public void GetNameAndMembers(Guid conversationId, Action<string, List<Guid>> onSuccess, Action onError)
		{
			_dbCollectionHandler.AddJob((ILiteCollection<DbConversation> dbCollection) =>
			{
				var convo = dbCollection.FindById(conversationId);
				if (convo == null)
					Task.Run(() => { onError.Invoke(); });
				else
					Task.Run(() => { onSuccess.Invoke(convo.Name, convo.Members); });
			});
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="conversationId"></param>
		/// <param name="onSuccess"></param>
		/// <param name="onError"></param>
		public void GetMultipleNameAndMembers(List<Guid> conversationIds, Action<List<(Guid, string, List<Guid>)>> onDone)
		{
			_dbCollectionHandler.AddJob((ILiteCollection<DbConversation> dbCollection) =>
			{
				var convos = new List<(Guid, string, List<Guid>)>();
				foreach(var id in conversationIds)
				{
					var convo = dbCollection.FindById(id);
					if (convo != null)
						convos.Add((id, convo.Name, convo.Members));
				}
				Task.Run(() => { onDone.Invoke(convos); });
			});
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="conversationId"></param>
		/// <param name="onSuccess"></param>
		/// <param name="onError"></param>
		public void GetName(Guid conversationId, Action<string> onSuccess, Action onError)
		{
			_dbCollectionHandler.AddJob((ILiteCollection<DbConversation> dbCollection) =>
			{
				var convo = dbCollection.FindById(conversationId);
				if (convo == null)
					Task.Run(() => { onError.Invoke(); });
				else
					Task.Run(() => { onSuccess.Invoke(convo.Name); });
			});
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="conversationId"></param>
		/// <param name="newName"></param>
		/// <param name="onSuccess"></param>
		/// <param name="onError"></param>
		public void SetName(Guid conversationId, string newName, Action onSuccess, Action onError)
		{
			_dbCollectionHandler.AddJob((ILiteCollection<DbConversation> dbCollection) =>
			{
				var convo = dbCollection.FindById(conversationId);
				if (convo == null)
					Task.Run(() => { onError.Invoke(); });
				else
				{
					convo.Name = newName;
					dbCollection.Update(convo);
					Task.Run(() => { onSuccess.Invoke(); });
				}
			});
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="conversationId"></param>
		/// <param name="memberId"></param>
		/// <param name="onSuccess"></param>
		/// <param name="onError"></param>
		public void MemberAdd(Guid conversationId, Guid memberId, Action onSuccess, Action onError)
		{
			_dbCollectionHandler.AddJob((ILiteCollection<DbConversation> dbCollection) =>
			{
				var convo = dbCollection.FindById(conversationId);
				if (convo == null || convo.Members.Contains(memberId))
					Task.Run(() => { onError.Invoke(); });
				else
				{
					convo.Members.Add(memberId);
					dbCollection.Update(convo);
					Task.Run(() => { onSuccess.Invoke(); });
				}
			});
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="conversationId"></param>
		/// <param name="memberId"></param>
		/// <param name="onSuccess"></param>
		/// <param name="onError"></param>
		public void MemberRemove(Guid conversationId, Guid memberId, Action onSuccess, Action onError)
		{
			_dbCollectionHandler.AddJob((ILiteCollection<DbConversation> dbCollection) =>
			{
				var convo = dbCollection.FindById(conversationId);
				if (convo == null || !convo.Members.Contains(memberId))
					Task.Run(() => { onError.Invoke(); });
				else
				{
					convo.Members.Remove(memberId);
					if (convo.Members.Count == 0)
						dbCollection.DeleteMany(c => c.Id == convo.Id);
					else
						dbCollection.Update(convo);
					Task.Run(() => { onSuccess.Invoke(); });
				}
			});
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="conversationId"></param>
		/// <param name="onSuccess"></param>
		/// <param name="onError"></param>
		public void MemberList(Guid conversationId, Action<List<Guid>> onSuccess, Action onError)
		{
			_dbCollectionHandler.AddJob((ILiteCollection<DbConversation> dbCollection) =>
			{
				var convo = dbCollection.FindById(conversationId);
				if (convo == null)
					Task.Run(() => { onError.Invoke(); });
				else
					Task.Run(() => { onSuccess.Invoke(convo.Members); });
			});
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="conversationId"></param>
		/// <param name="memberFrom"></param>
		/// <param name="content"></param>
		/// <param name="onSuccess"></param>
		/// <param name="onError"></param>
		public void AddMessage(Guid conversationId, Guid memberFrom, string content, Action<ConvMessage> onSuccess, Action<string> onError)
		{
			_dbCollectionHandler.AddJob((ILiteCollection<DbConversation> dbCollection) =>
			{
				var convo = dbCollection.FindById(conversationId);
				if (convo == null)
				{
					Task.Run(() => { onError.Invoke("Conversation doesnt exist"); });
					return;
				}

				if (!convo.Members.Contains(memberFrom))
				{
					Task.Run(() => { onError.Invoke("Not part of conversation!"); });
					return;
				}

				var msg = new ConvMessage()
				{
					id = Guid.NewGuid(),
					usrId = memberFrom,
					utcTime = DateTime.UtcNow,
					content = content,
				};

				convo.Messages.Add(msg);

				dbCollection.Update(convo);

				Task.Run(() => { onSuccess.Invoke(msg); });
			});
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="conversationId"></param>
		/// <param name="offset"></param>
		/// <param name="nMessages"></param>
		/// <param name="onSuccess"></param>
		/// <param name="onError"></param>
		public void GetMessages(Guid conversationId, ulong offset, ulong nMessages, Action<List<ConvMessage>> onSuccess, Action onError)
		{
			_dbCollectionHandler.AddJob((ILiteCollection<DbConversation> dbCollection) =>
			{
				var convo = dbCollection.FindById(conversationId);
				if (convo == null ||
					convo.Messages.Count < 0 ||
					offset > (ulong)convo.Messages.Count)
				{
					Task.Run(() => { onError.Invoke(); });
				}
				else
				{
					nMessages = Math.Min((uint)convo.Messages.Count - offset, nMessages);

					Task.Run(() => { onSuccess.Invoke(convo.Messages.GetRange((int)offset, (int)nMessages)); });
				}
			});
		}
	}
}
