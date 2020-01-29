using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CollarControl
{
	public static class Conversation_RequestHandler
	{
		public static void Dispatch(RuntimeUser thisUser, Connection client, RequestMethod method, Guid requestId, string payload)
		{
			switch (method)
			{
				case RequestMethod.GET:
					Get(thisUser, client, requestId, payload);
					break;
				case RequestMethod.POST:
					Post(thisUser, client, requestId, payload);
					break;
				case RequestMethod.DELETE:
					Delete(thisUser, client, requestId, payload);
					break;
				default:
					Program.SimpleClientResponse(client, requestId, ResponseCode.FORBIDDEN, "Not a valid method for this request");
					break;
			}
		}
		static void Get(RuntimeUser thisUser, Connection client, Guid requestId, string payload)
		{
			List<ServerPayloads.Conversation> conversations = new List<ServerPayloads.Conversation>();
			foreach (Guid id in thisUser.Conversations)
			{
				ServerPayloads.Conversation conv = new ServerPayloads.Conversation()
				{
					id = id,
					name = Program.conversationAPI.GetName(id),
					members = Program.conversationAPI.MemberList(id),
				};

				conversations.Add(conv);
			}

			ServerPackage messageObject = new ServerPackage()
			{
				code = ResponseCode.OK,
				type = ResponseDataType.CONVERSATION_LIST,
				requestId = requestId,
				payload = JsonConvert.SerializeObject(conversations),
			};

			string jsonMessage = messageObject.Serialize();

			client.SendMessage(jsonMessage);
		}
		static void Post(RuntimeUser thisUser, Connection client, Guid requestId, string payload)
		{
			List<Guid> request;
			try
			{
				request = JsonConvert.DeserializeObject<List<Guid>>(payload);
			}
			catch (Exception)
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, "Invalid payload");
				return;
			}

			var conversationName = thisUser.Username;
			var conversationMembers = new List<RuntimeUser>();
			for (int i = 0; i < request.Count; i++)
			{
				var user = Program.userAPI.GetById(request[i]);
				if (user != null)
				{
					conversationMembers.Add(user);
					conversationName += $", {user.Username}";
				}
				else
				{
					request.RemoveAt(i--);
				}
			}

			Guid conversationId = Program.conversationAPI.Add(conversationName, request);

			ServerPayloads.Conversation conversationResponse = new ServerPayloads.Conversation()
			{
				id = conversationId,
				name = conversationName,
				members = request,
			};

			ServerPackage message = new ServerPackage()
			{
				code = ResponseCode.OK,
				type = ResponseDataType.CONVERSATION,
				requestId = requestId,
				payload = conversationResponse.Serialize(),
			};
			client.SendMessage(message.Serialize());
			message.code = ResponseCode.UPDATE_DATA;
			message.requestId = Guid.Empty;

			foreach (Guid id in request)
			{
				if (id != thisUser.Id)
					Program.userAPI.GetById(id)?.SendMessage(message.Serialize());
			}
		}
		static void Delete(RuntimeUser thisUser, Connection client, Guid requestId, string payload)
		{
			// TODO implement me
			throw new NotImplementedException($"Conversation_RequestHandler.Delete({thisUser},{client},{requestId},{payload})");
		}
	}
}
