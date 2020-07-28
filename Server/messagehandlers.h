#ifndef MESSAGEHANDLERS_H
#define MESSAGEHANDLERS_H

#include <vector>

#include <instance.h>
#include <thorq_payload.h>

void handleMessageVersion(ThorQ::Instance* instance, const thorq_payload_t& payload);
void handleMessageCrypto(ThorQ::Instance* instance, const thorq_payload_t& payload);
void handleMessageAuth(ThorQ::Instance* instance, const thorq_payload_t& payload);
void handleMessageHeartbeat(ThorQ::Instance* instance);
void handleMessageCommand(ThorQ::Instance* instance, const thorq_payload_t& payload);
void handleMessageCommandAck(ThorQ::Instance* instance, const thorq_payload_t& payload);
void handleMessageNotification(ThorQ::Instance* instance, const thorq_payload_t& payload);
void handleMessageCollar(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);

#endif // MESSAGEHANDLERS_H
