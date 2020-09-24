#ifndef MESSAGEHANDLERS_H
#define MESSAGEHANDLERS_H

#include <vector>
#include <cstdint>

#include <typedefs.h>

void handleMessageHeartbeat(ThorQ::Session* instance, const std::vector<std::uint8_t>& message);
void handleMessageVersion(ThorQ::Session* instance, const std::vector<std::uint8_t>& message);
void handleMessageCrypto(ThorQ::Session* instance, const std::vector<std::uint8_t>& message);
void handleMessageSystemID(ThorQ::Session* instance, const std::vector<std::uint8_t>& message);
void handleMessageRegKey(ThorQ::Session* instance, const std::vector<std::uint8_t>& message);
void handleMessageAccount(ThorQ::Session* instance, const std::vector<std::uint8_t>& message);
void handleMessageSession(ThorQ::Session* instance, const std::vector<std::uint8_t>& message);
void handleMessageFriend(ThorQ::Session* instance, const std::vector<std::uint8_t>& message);
void handleMessageRoom(ThorQ::Session* instance, const std::vector<std::uint8_t>& message);
void handleMessageModeration(ThorQ::Session* instance, const std::vector<std::uint8_t>& message);
void handleMessageAnnouncement(ThorQ::Session* instance, const std::vector<std::uint8_t>& message);
void handleMessageCollar(ThorQ::Session* instance, const std::vector<std::uint8_t>& message);
void handleMessageAck(ThorQ::Session* instance, const std::vector<std::uint8_t>& message);

#endif // MESSAGEHANDLERS_H
