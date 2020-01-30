using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using HeavenLib.Connectivity;
using CollarLib;

namespace CollarControl
{
	public static class Message_RequestHandler
	{
		public static void Dispatch(RuntimeUser thisUser, HostConnection client, RequestMethod method, Guid requestId, string payload)
		{
			switch (method)
			{
				case RequestMethod.GET:
					Get(thisUser, client, requestId, payload);
					break;
				case RequestMethod.SET:
					Set(thisUser, client, requestId, payload);
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
		static void Get(RuntimeUser thisUser, HostConnection client, Guid requestId, string payload)
		{
			CollarLib.ClientPayloads.ConversationGetRequest request;
			try
			{
				request = JsonConvert.DeserializeObject<CollarLib.ClientPayloads.ConversationGetRequest>(payload);
			}
			catch (Exception)
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, "Invalid payload");
				return;
			}

			var messages = Program.conversationAPI.GetMessages(request.conversationId, request.offset, request.nMessages);

			Response serv = new Response()
			{
				code = ResponseCode.OK,
				type = ResponseDataType.MESSAGE_LIST,
				requestId = requestId,
				payload = JsonConvert.SerializeObject(messages),
			};
			client.SendMessage(serv.Serialize());
		}
		static void Set(RuntimeUser thisUser, HostConnection client, Guid requestId, string payload)
		{
			// TODO implement me
			throw new NotImplementedException($"Message_RequestHandler.Set({thisUser},{client},{requestId},{payload})");
		}
		static void Post(RuntimeUser thisUser, HostConnection client, Guid requestId, string payload)
		{
			CollarLib.ClientPayloads.MessagePostRequest request;
			try
			{
				request = CollarLib.ClientPayloads.MessagePostRequest.Deserialize(payload);
			}
			catch (Exception)
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, "Invalid payload");
				return;
			}

			ConvMessage? message = Program.conversationAPI.AddMessage(request.conversationId, thisUser.Id, request.content);

			if (message == null)
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.UNAUTHORIZED, "Not part of conversation");
				return;
			}

			List<Guid> members = Program.conversationAPI.MemberList(request.conversationId);

			Response serv = new Response()
			{
				code = ResponseCode.OK,
				type = ResponseDataType.MESSAGE,
				requestId = requestId,
				payload = message?.Serialize(),
			};
			client.SendMessage(serv.Serialize());
			serv.code = ResponseCode.UPDATE_DATA;
			serv.requestId = Guid.Empty;

			foreach (Guid id in members)
			{
				if (id != thisUser.Id)
					Program.userAPI.GetById(id)?.SendMessage(message?.Serialize());
			}
		}
		static void Delete(RuntimeUser thisUser, HostConnection client, Guid requestId, string payload)
		{
			// TODO implement me
			throw new NotImplementedException($"Message_RequestHandler.Delete({thisUser},{client},{requestId},{payload})");
		}
	}
}
