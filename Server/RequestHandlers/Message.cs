using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using HeavenLib.Connectivity;
using CollarLib;

namespace ThorQ
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

			Program.conversationAPI.GetMessages(request.conversationId, request.offset, request.nMessages,
				(messages)=>
				{
					Response serv = new Response()
					{
						code = ResponseCode.OK,
						type = ResponseType.MESSAGE_LIST,
						requestId = requestId,
						payload = JsonConvert.SerializeObject(messages),
					};
					client.SendEncrypted(Encoding.UTF8.GetBytes(serv.Serialize()));
				},
				()=>
				{
					// TODO implement me
				});
;
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

			Program.conversationAPI.AddMessage(request.conversationId, thisUser.Id, request.content, (message) =>
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.OK, "Message sent!");

				Program.conversationAPI.MemberList(request.conversationId, (memberIds) =>
				{
					Response resp = new Response()
					{
						code = ResponseCode.UPDATE_DATA,
						type = ResponseType.MESSAGE,
						requestId = Guid.Empty,
						payload = message.Serialize(),
					};

					foreach (Guid id in memberIds)
					{
						if (Program.initializedUsers.TryGetValue(id, out var user))
						{
							user.SendMessage(resp.Serialize());
						}
					}
				},
				() =>
				{
					Program.SimpleClientResponse(client, requestId, ResponseCode.UNAUTHORIZED, "Conversation doesnt exist");
				});
			},
			(err) =>
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.UNAUTHORIZED, err);
			});
		}
		static void Delete(RuntimeUser thisUser, HostConnection client, Guid requestId, string payload)
		{
			// TODO implement me
			throw new NotImplementedException($"Message_RequestHandler.Delete({thisUser},{client},{requestId},{payload})");
		}
	}
}
