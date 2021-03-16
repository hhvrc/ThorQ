#ifndef TYPEDEFS_GLOBAL_H
#define TYPEDEFS_GLOBAL_H

namespace ThorQ {
namespace Encoding {
struct MessageHeader;
} // Encoding
namespace Serialization {
class Message;
} // Serialization
namespace Networking {
class TcpConnection;
class UdpConnection;
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
