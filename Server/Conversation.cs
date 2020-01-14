using System;
using System.Collections.Generic;
using System.Text;

namespace CollarControl
{
	[Serializable]
	class Conversation
	{
		[Serializable]
		class Message
		{
			Guid userId;
			Guid messageId;
			DateTime sendDate;

			String content;
		}

		Guid id;
		List<Guid> members;
		List<Message> messages;

	}
}
