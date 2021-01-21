#ifndef MESSAGE_H
#define MESSAGE_H

#include <cstdint>
#include <memory>

#include <QMetaType>

#include "enet.h"

namespace ThorQ {
namespace Networking {
class Message
{
public:
    Message();
    Message(const ENetEvent& event);
    Message(ENetPacket* packet, std::uint8_t channelID);
    Message(const ThorQ::Networking::Message& other);
    ~Message();

    ENetPacket* packet() const;
    std::uint8_t channelID() const;

    ThorQ::Networking::Message& operator=(const ThorQ::Networking::Message& other);
private:
    std::shared_ptr<ENetPacket> m_packet;
    std::uint8_t m_channelID;
};
}
}
Q_DECLARE_METATYPE(ThorQ::Networking::Message)
Q_DECLARE_METATYPE(ENetPacket*)
Q_DECLARE_METATYPE(ENetPacket)
Q_DECLARE_METATYPE(ENetEvent*)
Q_DECLARE_METATYPE(ENetEvent)
Q_DECLARE_METATYPE(ENetPeer*)
Q_DECLARE_METATYPE(ENetPeer)
Q_DECLARE_METATYPE(std::uint64_t)
Q_DECLARE_METATYPE(std::uint32_t)
Q_DECLARE_METATYPE(std::uint16_t)
Q_DECLARE_METATYPE(std::uint8_t)
Q_DECLARE_METATYPE(std::int64_t)
Q_DECLARE_METATYPE(std::int32_t)
Q_DECLARE_METATYPE(std::int16_t)
Q_DECLARE_METATYPE(std::int8_t)

#endif // MESSAGE_H
