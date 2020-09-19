#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QString>
#include <vector>

#include "enums.h"

/** @enum THORQ_NOTIFICATION
 */
enum THORQ_NOTIFICATION_TYPE
{
    THORQ_NOTIFICATION_USER_ACTIVITY,

    THORQ_NOTIFICATION_USER_OFFLINE,
    THORQ_NOTIFICATION_USER_OFFLINE_LOS,
    THORQ_NOTIFICATION_USER_OFFLINE_TIMEOUT,
};

/**
 * @brief thorq_payload_notification_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_notification_is_valid(const std::vector<std::uint8_t>& payload)
{
    if (payload.size() >= 2)
    {
        switch (payload[0]) {
        case THORQ_NOTIFICATION_USER_OFFLINE:
        case THORQ_NOTIFICATION_USER_OFFLINE_LOS:
        case THORQ_NOTIFICATION_USER_OFFLINE_TIMEOUT:
            return true;
        case THORQ_NOTIFICATION_USER_ACTIVITY:
            return payload.size() >= 3;
        default:
            return false;
        }
    }

    return false;
}

/**
 * @brief thorq_payload_notification_pack
 * @param payload
 * @param type
 * @param message
 */
inline void thorq_payload_notification_pack(std::vector<std::uint8_t>& payload, const THORQ_NOTIFICATION_TYPE& type, const QString& message)
{
	QByteArray messageBytes = message.toUtf8();

	payload.resize(2 + messageBytes.size());

    payload[0] = THORQ_PAYLOAD_ID_NOTIFICATION;
	payload[1] = static_cast<quint8>(type);

	memcpy(payload.data() + 2, messageBytes.data(), messageBytes.size());
}

/**
 * @brief thorq_payload_notification_pack
 * @param payload
 * @param type
 * @param message
 * @param data
 */
inline void thorq_payload_notification_pack(std::vector<std::uint8_t>& payload, const THORQ_NOTIFICATION_TYPE& type, const QString& message, quint8 data)
{
	QByteArray messageBytes = message.toUtf8();

	payload.resize(3 + messageBytes.size());

    payload[0] = THORQ_PAYLOAD_ID_NOTIFICATION;
	payload[1] = static_cast<quint8>(type);

	memcpy(payload.data() + 2, messageBytes.data(), messageBytes.size());

	payload[2 + messageBytes.size()] = data;
}

inline void thorq_payload_notification_get_type(const std::vector<std::uint8_t>& payload, THORQ_NOTIFICATION_TYPE& type)
{
    type = static_cast<THORQ_NOTIFICATION_TYPE>(payload[1]);
}

/**
 * @brief thorq_payload_notification_get_message
 * @param payload
 * @param message
 */
inline void thorq_payload_notification_get_message(const std::vector<std::uint8_t>& payload, QString& message)
{
	message.fromUtf8((const char*)payload.data() + 2, (int)payload.size() - 2);
}

/**
 * @brief thorq_payload_notification_get_message
 * @param payload
 * @param message
 * @param data
 */
inline void thorq_payload_notification_get_message_and_data(const std::vector<std::uint8_t>& payload, QString& message, quint8 data)
{
	message.fromUtf8((const char*)payload.data() + 2, (int)payload.size() - 3);

    data = payload[payload.size() - 1];
}

#endif // NOTIFICATION_H
