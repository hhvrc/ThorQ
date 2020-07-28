#ifndef MESSAGEHANDLERS_H
#define MESSAGEHANDLERS_H

#include <instance.h>
#include <thorq_payload.h>

static void handleMessageVersion(ThorQ::Instance* isntance, const thorq_payload_t& payload);
static void handleMessageCrypto(ThorQ::Instance* isntance, const thorq_payload_t& payload);
static void handleMessageAuth(ThorQ::Instance* isntance, const thorq_payload_t& payload);
static void handleMessageHeartbeat(ThorQ::Instance* isntance, const thorq_payload_t& payload);
static void handleMessageCommand(ThorQ::Instance* isntance, const thorq_payload_t& payload);
static void handleMessageCommandAck(ThorQ::Instance* isntance, const thorq_payload_t& payload);
static void handleMessageNotification(ThorQ::Instance* isntance, const thorq_payload_t& payload);
static void handleMessageCollar(ThorQ::Instance* isntance, const thorq_payload_t& payload);

#endif // MESSAGEHANDLERS_H
