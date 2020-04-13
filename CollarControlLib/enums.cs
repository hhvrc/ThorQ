using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CollarLib
{
	public enum RequestType
	{
		Ping,		// Connection check
		Register,	// Get key to send to partner
		Request,	// Send request to partner
		Accept,		// Accept incoming request
		Deny		// Deny incoming request
	}
	public enum ResponseType
	{
		Ping,       // Connection Check
		Ok,         // Success
		Key,		// Key to give to partner
		Request,    // Icoming request to connect
		P2PInfo,	// Connection info (request accepted)
		Denied,		// Request to connect was denied
		Error		// Serverside error
	}
}
