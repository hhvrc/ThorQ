#ifndef TYPEDEFS_GLOBAL_H
#define TYPEDEFS_GLOBAL_H

namespace ThorQ {
namespace Networking {
class Client;
class Server;
class Connection;
class ConnectionHandlerInterface;
namespace Tcp { class Client; class Server; class Connection; }
namespace Udp { class Client; class Server; class Connection; }
struct MessageHeader;
struct Message;
struct IncomingMessage;
} // Networking
namespace Crypto {
class Signer;
class Encryption;
} // Crypto
class Uuid;
class Version;
} // ThorQ

namespace flatbuffers {
class Verifier;
}

#endif // TYPEDEFS_GLOBAL_H
