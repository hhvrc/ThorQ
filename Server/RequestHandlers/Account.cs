using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CollarControl
{
	public static class Account_RequestHandler
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
			if (thisUser != null)
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.NOPE, "Already logged in");
				return;
			}

			ClientPayloads.AccountGetRequest request = ClientPayloads.AccountGetRequest.Deserialize(payload);

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

			RuntimeUser user = Program.userAPI.GetByName(request.username);
			if (user == null || !user.VerifyPassword(request.password))
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.UNAUTHORIZED, "Invalid username/password");
				return;
			}

			if (!user.HasConnection(client))
			{
				client.OnClientDisconnected -= Program.OnClientDisconnected;
				client.OnMessageReceived -= Program.OnClientMessageReceived;
				Program.nonAuthedConnections.Remove(client);
				user.AddConnection(client);
			}

			ServerPayloads.Account response = new ServerPayloads.Account();
			response.username = user.Username;
			response.state = user.state;
			response.status = user.status;
			response.email = user.Email;

			ServerPackage message = new ServerPackage()
			{
				code = ResponseCode.OK,
				type = ResponseDataType.ACCOUNT,
				requestId = requestId,
				payload = request.Serialize(),
			};

			client.SendMessage(message.Serialize());

			Console.WriteLine("[Client] Logged in"); // DEBUG
		}
		static void Post(RuntimeUser thisUser, Connection client, Guid requestId, string payload)
		{
			if (thisUser != null)
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.NOPE, "Already logged in");
				return;
			}

			ClientPayloads.AccountPostRequest request = ClientPayloads.AccountPostRequest.Deserialize(payload);

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

			if (Program.userAPI.EmailExists(request.email))
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.NOPE, "Email taken");
				return;
			}

			thisUser = Program.userAPI.TryAdd(request.username, request.password, request.email);
			if (thisUser == null)
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.NOPE, "Username/Email taken");
				return;
			}
			thisUser.IsOnlineChanged += Program.OnUserOnlineChanged;
			thisUser.MessageReceived += Program.OnUserMessageReceived;

			client.OnClientDisconnected -= Program.OnClientDisconnected;
			client.OnMessageReceived -= Program.OnClientMessageReceived;

			Program.nonAuthedConnections.Remove(client);

			thisUser.AddConnection(client);
			Program.userAPI.TryUpdate(thisUser);

			Program.SimpleClientResponse(client, requestId, ResponseCode.CREATED, "Account created");
			Console.WriteLine("[Client] Registered");
		}
		static void Delete(RuntimeUser thisUser, Connection client, Guid requestId, string payload)
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
			
			if (!thisUser.VerifyPassword(payload))
			{
				Program.SimpleClientResponse(client, requestId, ResponseCode.UNAUTHORIZED, "Invalid password");
				return;
			}

			thisUser.RemoveConnection(client);
			Program.userAPI.TryRemove(thisUser.Id);

			Program.nonAuthedConnections.Add(client);

			client.OnClientDisconnected += Program.OnClientDisconnected;
			client.OnMessageReceived += Program.OnClientMessageReceived;

			Program.SimpleClientResponse(client, requestId, ResponseCode.DELETED, "Account deleted");
			Console.WriteLine("[Client] Deleted account"); // DEBUG
		}
	}
}
