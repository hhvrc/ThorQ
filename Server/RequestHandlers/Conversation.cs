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
	public static class Conversation_RequestHandler
	{
		public static void Dispatch(RuntimeUser thisUser, HostConnection client, RequestMethod method, Guid requestId, string payload)
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
		static void Get(RuntimeUser thisUser, HostConnection client, Guid requestId, string _)
		{
			Program.userAPI.GetById(thisUser.Id, (dbUser) =>
			{
				Program.conversationAPI.GetMultipleNameAndMembers(dbUser.Conversations, (conversations) =>
				{
					var convos = new List<CollarLib.ServerPayloads.Conversation>();
					foreach (var conversation in conversations)
					{
						convos.Add(new CollarLib.ServerPayloads.Conversation
						{
							Id = conversation.Item1,
							Name = conversation.Item2,
							Members = conversation.Item3
						});
					}

					var messageObject = new CollarLib.Response()
					{
						code = ResponseCode.OK,
						type = ResponseType.CONVERSATION_LIST,
						requestId = requestId,
						payload = JsonConvert.SerializeObject(convos),
					};
					client.SendEncrypted(Encoding.UTF8.GetBytes(messageObject.Serialize()));
				});
			},
			(err) =>
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, err);
			});
		}
		static void Post(RuntimeUser thisUser, HostConnection client, Guid requestId, string payload)
		{
			List<Guid> memberIds;
			try
			{
				memberIds = JsonConvert.DeserializeObject<List<Guid>>(payload);
			}
			catch (Exception)
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, "Invalid payload");
				return;
			}


			Program.userAPI.LimitIdsToUserFriendslist(thisUser.Id, memberIds, (friendMemberIds) =>
			{
				if (memberIds.Count == 0)
				{
					Program.SimpleClientResponse(client, requestId, ResponseCode.NOPE, "Cannot add non-friended users to conversation!");
					return;
				}

				memberIds.Add(thisUser.Id);

				Program.userAPI.GetMultipleById(friendMemberIds, (friendMembers) =>
				{
					String convName = "";
					foreach (var member in friendMembers)
						convName += $", {member.Username}";

					Program.conversationAPI.Add(convName, friendMemberIds, (conversationId) =>
					{
						Program.userAPI.AddConversationToUsers(friendMemberIds, conversationId);

						Program.SimpleClientResponse(client, requestId, ResponseCode.OK, "Conversation created!");

						foreach (var member in friendMembers)
						{
							if (Program.initializedUsers.TryGetValue(member.Id, out var user))
							{
								List<Guid> convoList = new List<Guid>();
								convoList.AddRange(member.Conversations);
								convoList.Add(conversationId);

								var messageObject = new CollarLib.Response()
								{
									code = ResponseCode.UPDATE_DATA,
									type = ResponseType.CONVERSATION_LIST,
									requestId = Guid.Empty,
									payload = JsonConvert.SerializeObject(convoList),
								};
								user.SendMessage(messageObject.Serialize());
							}
						}
					});
				});
			},
			(err) =>
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, err);
			});
		}
		static void Delete(RuntimeUser thisUser, HostConnection client, Guid requestId, string payload)
		{
			// TODO implement me
			throw new NotImplementedException($"Conversation_RequestHandler.Delete({thisUser},{client},{requestId},{payload})");
		}
	}
}
