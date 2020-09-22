#ifndef MESSAGEHANDLERS_H
#define MESSAGEHANDLERS_H

#include <vector>
#include <cstdint>

#include <typedefs.h>

void handleMessageVersion(ThorQ::Session* instance, const std::vector<std::uint8_t>& message);
void handleMessageCrypto(ThorQ::Session* instance, const std::vector<std::uint8_t>& message);
void handleMessageAuth(ThorQ::Session* instance, const std::vector<std::uint8_t>& message);
void handleMessageHeartbeat(ThorQ::Session* instance);
void handleMessageLogin(ThorQ::Session* instance, const std::vector<std::uint8_t>& message);
void handleMessageLogout(ThorQ::Session* instance);
void handleMessageCommand(ThorQ::Session* instance, const std::vector<std::uint8_t>& message);
void handleMessageCommandAck(ThorQ::Session* instance, const std::vector<std::uint8_t>& message);
void handleMessageCollar(ThorQ::Session* instance, const std::vector<std::uint8_t>& message);

#endif // MESSAGEHANDLERS_H
