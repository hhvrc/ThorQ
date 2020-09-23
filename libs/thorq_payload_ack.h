#ifndef THORQ_PAYLOAD_ACK_H
#define THORQ_PAYLOAD_ACK_H

#include <QString>
#include <QByteArray>

#include "enums.h"

/**
 * @brief thorq_payload_ack_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_ack_is_valid(const QByteArray& payload)
{
	return payload.size() >= 3 && payload[0] == THORQ_PAYLOAD_ID_ACK && payload[1] < THORQ_PAYLOAD_ID_ACK; // payload id that gets acked can be anything else than an ack
}

/**
 * @brief thorq_payload_ack_pack
 * @param payload
 * @param id
 * @param cmd
 */
inline void thorq_payload_ack_pack(QByteArray& payload, THORQ_PAYLOAD_ID id, std::uint8_t cmd)
{
	payload.resize(3);
	payload[0] = THORQ_PAYLOAD_ID_ACK;
	payload[1] = id;
	payload[2] = cmd;
}

/**
 * @brief thorq_payload_ack_pack
 * @param payload
 * @param cmd
 * @param id
 * @param string
 */
inline void thorq_payload_ack_pack(QByteArray& payload, std::uint8_t cmd, THORQ_PAYLOAD_ID id, const QString& string)
{
	QByteArray stringBytes = string.toUtf8();

	payload.resize(3 + stringBytes.size());
	payload[0] = THORQ_PAYLOAD_ID_ACK;
	payload[1] = id;
	payload[2] = cmd;

	memcpy(payload.data() + 3, stringBytes.data(), stringBytes.size());
}

/**
 * @brief thorq_payload_ack_get_id
 * @param payload
 * @param id
 */
inline void thorq_payload_ack_get_id(const QByteArray& payload, THORQ_PAYLOAD_ID& id)
{
	id = static_cast<THORQ_PAYLOAD_ID>(payload[1]);
}

/**
 * @brief thorq_payload_ack_get_cmd
 * @param payload
 * @param cmd
 */
inline void thorq_payload_ack_get_cmd(const QByteArray& payload, std::uint8_t& cmd)
{
	cmd = payload[2];
}

/**
 * @brief thorq_payload_ack_get_message
 * @param payload
 * @param string
 */
inline void thorq_payload_ack_get_message(const QByteArray& payload, QString& string)
{
	string.resize(payload.size() - 3);

	memcpy(string.data(), payload.data() + 3, payload.size() - 3);
}

#endif // THORQ_PAYLOAD_ACK_H
