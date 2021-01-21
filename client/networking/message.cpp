#include "message.h"

ThorQ::Networking::Message::Message()
    : m_packet(nullptr)
    , m_channelID(0)
{
}

ThorQ::Networking::Message::Message(const ENetEvent &event)
    : m_packet(event.packet)
    , m_channelID(event.channelID)
{
}

ThorQ::Networking::Message::Message(const ThorQ::Networking::Message &other)
    : m_packet(other.m_packet)
    , m_channelID(other.m_channelID)
{
}

ThorQ::Networking::Message::Message(ENetPacket* packet, std::uint8_t channelID)
    : m_packet(packet, [](ENetPacket* packet){ enet_packet_destroy(packet); })
    , m_channelID(channelID)
{
}

ThorQ::Networking::Message::~Message()
{
}

ENetPacket* ThorQ::Networking::Message::packet() const
{
    return m_packet.get();
}

std::uint8_t ThorQ::Networking::Message::channelID() const
{
    return m_channelID;
}

ThorQ::Networking::Message& ThorQ::Networking::Message::operator=(const ThorQ::Networking::Message& other)
{
    m_packet = other.m_packet;
    m_channelID = other.m_channelID;
    return *this;
}
