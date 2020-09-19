#ifndef MESSAGEHANDLERS_H
#define MESSAGEHANDLERS_H

#include <vector>
#include <cstdint>

#include <typedefs.h>

void handleMessageVersion(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
void handleMessageCrypto(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
void handleMessageAuth(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
void handleMessageHeartbeat(ThorQ::Instance* instance);
void handleMessageLogin(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
void handleMessageLogout(ThorQ::Instance* instance);
void handleMessageCommand(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
void handleMessageCommandAck(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
void handleMessageCollar(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);

#endif // MESSAGEHANDLERS_H
