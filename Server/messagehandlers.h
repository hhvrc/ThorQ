#ifndef MESSAGEHANDLERS_H
#define MESSAGEHANDLERS_H

#include <vector>
#include <cstdint>

#include "typedefs_server.h"

void handleMessageHeartbeat(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
void handleMessageVersion(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
void handleMessageCrypto(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
void handleMessageSystemID(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
void handleMessageAccount(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
void handleMessageRelation(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
void handleMessageSession(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
void handleMessageModeration(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
void handleMessageAnnouncement(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
void handleMessageCollar(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
void handleMessageAck(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);

#endif // MESSAGEHANDLERS_H
