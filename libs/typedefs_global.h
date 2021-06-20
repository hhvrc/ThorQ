#ifndef TYPEDEFS_GLOBAL_H
#define TYPEDEFS_GLOBAL_H

namespace ThorQ {

    namespace Encoding {
        struct MessageHeader;
    } // Encoding

    namespace Serialization {
        struct Message;
    } // Serialization

    namespace Networking {
        class TcpConnection;
        class UdpConnection;
    } // Networking

    namespace Crypto {
        class Signer;
        class Encryption;
    } // Crypto

    struct Uuid;
    struct Version;

} // ThorQ

namespace flatbuffers {
    class Verifier;
} // flatbuffers

#endif // TYPEDEFS_GLOBAL_H
