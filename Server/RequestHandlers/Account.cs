using CollarLib;
using HeavenLib;
using HeavenLib.Connectivity;
using System;
using System.Text;

namespace ThorQ
{
	public static class Account_RequestHandler
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
		static void Get(RuntimeUser thisUser, HostConnection client, Guid requestId, string payload)
		{
			if (thisUser != null)
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.NOPE, "Already logged in");
				return;
			}

			CollarLib.ClientPayloads.AccountGetRequest request = CollarLib.ClientPayloads.AccountGetRequest.Deserialize(payload);

			if (string.IsNullOrWhiteSpace(request.username))
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.INVALID_PARAMS, "Username cannot be empty");
				return;
			}

			if (string.IsNullOrWhiteSpace(request.password))
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.INVALID_PARAMS, "Password cannot be empty");
				return;
			}

			Program.userAPI.GetByName(request.username, (dbUser) =>
			{
				if (!dbUser.VerifyPassword(request.password))
				{
					Program.SimpleClientResponse(client, requestId, ResponseCode.UNAUTHORIZED, "Invalid username/password");
					return;
				}

				if (!Program.initializedUsers.TryGetValue(dbUser.Id, out thisUser))
				{
					thisUser = new RuntimeUser(dbUser.Id);
					Program.initializedUsers.TryAdd(dbUser.Id, thisUser);
				}

				if (thisUser.TryAddConnection(client))
				{
					client.OnClientDisconnected -= Program.OnClientDisconnected;
					client.OnMessageReceived -= Program.OnClientMessageReceived;
					Program.nonAuthedConnections.Remove(client);
				}

				Program.userAPI.GetMultipleById(dbUser.Friends, (users) => {
					Program.conversationAPI.GetMultipleNameAndMembers(dbUser.Conversations, (convos) => {

						var response = new CollarLib.ServerPayloads.AccountInstance(dbUser.Username, dbUser.Status, dbUser.Activity);

						foreach (var user in users) {
							var friend = new CollarLib.ServerPayloads.Friend();

							friend.UserId = user.Id;
							friend.Username = user.Username;
							friend.Activity = user.Activity;
							friend.Status = user.Status;

							response.Friends.Add(friend);
						}

						foreach (var blockedUser in dbUser.BlockedUsers)
						{
							response.BlockedUsers.Add(
								new CollarLib.ServerPayloads.BlockedUser(
									blockedUser.id,
									blockedUser.frozenUsername
									)
								);
						}

						response.FriendRequests = dbUser.FriendRequests;

						foreach (var convo in convos) {
							response.Conversations.Add(
								new CollarLib.ServerPayloads.Conversation(
									convo.Item1,
									convo.Item2,
									convo.Item3
									)
								);
						}

						Response message = new Response()
						{
							code = ResponseCode.OK,
							type = ResponseType.ACCOUNT,
							requestId = requestId,
							payload = response.Serialize(),
						};

						client.SendEncrypted(Encoding.UTF8.GetBytes(message.Serialize()));

						Console.WriteLine("[Client] Logged in"); // DEBUG
					});
				});
			},
			() =>
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.UNAUTHORIZED, "Invalid username/password");
			});
		}
		static void Post(RuntimeUser thisUser, HostConnection client, Guid requestId, string payload)
		{
			if (thisUser != null)
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.NOPE, "Already logged in");
				return;
			}

			CollarLib.ClientPayloads.AccountPostRequest request = CollarLib.ClientPayloads.AccountPostRequest.Deserialize(payload);

			if (string.IsNullOrWhiteSpace(request.email))
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.INVALID_PARAMS, "Email cannot be empty");
				return;
			}
			if (!ToolBox.IsValidEmail(request.email))
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.INVALID_PARAMS, "Email is invalid format");
				return;
			}

			if (string.IsNullOrWhiteSpace(request.username))
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.INVALID_PARAMS, "Username cannot be empty");
				return;
			}

			if (string.IsNullOrWhiteSpace(request.password))
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.INVALID_PARAMS, "Password cannot be empty");
				return;
			}

			Program.userAPI.AddUser(request.username, request.password, request.email,
				(DbUser dbUser) =>
				{
					thisUser = new RuntimeUser(dbUser.Id);

					thisUser.TryAddConnection(client);

					if (!Program.initializedUsers.TryAdd(thisUser.Id, thisUser))
						Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, "Server has encountered an error!");

					thisUser.IsOnlineChanged += Program.OnUserOnlineChanged;
					thisUser.MessageReceived += Program.OnUserMessageReceived;

					client.OnClientDisconnected -= Program.OnClientDisconnected;
					client.OnMessageReceived -= Program.OnClientMessageReceived;

					Program.nonAuthedConnections.Remove(client);

					thisUser.TryAddConnection(client);

					Program.SimpleClientResponse(client, requestId, ResponseCode.CREATED, "Account created");
					Console.WriteLine("[Client] Registered");
				},
				(String err) =>
				{
					Program.SimpleClientResponse(client, requestId, ResponseCode.NOPE, err);
				});

		}
		static void Delete(RuntimeUser thisUser, HostConnection client, Guid requestId, string payload)
		{
			if (thisUser == null)
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.UNAUTHORIZED, "Not logged in");
				return;
			}
			if (string.IsNullOrWhiteSpace(payload))
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.INVALID_PARAMS, "Password cannot be empty");
				return;
			}

			Program.userAPI.GetById(thisUser.Id, (dbUser) =>
			{
				if (!dbUser.VerifyPassword(payload))
				{
					Program.SimpleClientResponse(client, requestId, ResponseCode.UNAUTHORIZED, "Invalid password");
					return;
				}
				thisUser.RemoveConnection(client);
				Program.userAPI.RemoveUser(thisUser.Id, () =>
				{
					Program.nonAuthedConnections.Add(client);

					client.OnClientDisconnected += Program.OnClientDisconnected;
					client.OnMessageReceived += Program.OnClientMessageReceived;

					Program.SimpleClientResponse(client, requestId, ResponseCode.DELETED, "Account deleted");
					Console.WriteLine("[Client] Deleted account"); // DEBUG
				},
				() =>
				{
					Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, "Server error");
					Console.WriteLine("[Client] Error deleting account"); // DEBUG
				});
			},
			(err) =>
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.ERROR, "Server error");
				Console.WriteLine("[Client] Error deleting account"); // DEBUG
			});



		}
	}
}
