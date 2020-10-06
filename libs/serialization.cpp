#include "serialization.h"

#include "thorq_message.h"

void thorq_payload_serialization_prealloc(std::vector<std::uint8_t> &payload)
{
    payload.reserve(THORQ_MESSAGE_LEN);
}

bool thorq_payload_serialization_is_valid(const std::vector<std::uint8_t> &payload)
{
    return payload.size() >= 2;
}

std::uint8_t thorq_payload_serialization_get_id(const std::vector<std::uint8_t> &payload)
{
    return payload[0];
}

std::uint8_t thorq_payload_serialization_get_cmd(const std::vector<std::uint8_t> &payload)
{
    return payload[1];
}

void thorq_payload_serialization_pack(std::vector<std::uint8_t> &payload, THORQ_PAYLOAD_ID id, std::uint8_t cmd)
{
    payload.resize(2);
    payload[0] = id;
    payload[1] = cmd;
}

void thorq_payload_serialization_bytes_pack(std::vector<std::uint8_t> &payload, THORQ_PAYLOAD_ID id, std::uint8_t cmd, std::initializer_list<std::uint8_t> bytes)
{
    payload.resize(2 + bytes.size());
    payload[0] = id;
    payload[1] = cmd;
    memcpy(payload.data() + 2, bytes.begin(), bytes.size());
}

std::uint8_t thorq_payload_serialization_bytes_get(const std::vector<std::uint8_t> &payload, std::size_t index)
{
    return payload[2 + index];
}

const std::uint8_t *thorq_payload_serialization_bytes_unpack(const std::vector<std::uint8_t> &payload, std::size_t &size)
{
    size = payload.size() - 2;
    return payload.data() + 2;
}

void thorq_payload_serialization_pack_bytearray(std::vector<std::uint8_t> &payload, THORQ_PAYLOAD_ID id, std::uint8_t cmd, const QByteArray &data)
{
    payload.resize(2 + data.size());
    payload[0] = id;
    payload[1] = cmd;

    memcpy(payload.data() + 2, data.data(), data.size());
}

void thorq_payload_serialization_unpack_bytearray(const std::vector<std::uint8_t> &payload, QByteArray &data)
{
    data.resize(payload.size() - 2);
    memcpy(data.data(), payload.data() + 2, payload.size() - 2);
}

void thorq_payload_serialization_pack_1string(std::vector<std::uint8_t> &payload, THORQ_PAYLOAD_ID id, std::uint8_t cmd, const QString &string)
{
    QByteArray stringBytes = string.toUtf8();

    payload.resize(2 + stringBytes.size());
    payload[0] = id;
    payload[1] = cmd;

    memcpy(payload.data() + 2, stringBytes.data(), stringBytes.size());
}

void thorq_payload_serialization_pack_1string(std::vector<std::uint8_t> &payload, THORQ_PAYLOAD_ID id, std::uint8_t cmd, const QStringRef &string)
{
    QByteArray stringBytes = string.toUtf8();

    payload.resize(2 + stringBytes.size());
    payload[0] = id;
    payload[1] = cmd;

    memcpy(payload.data() + 2, stringBytes.data(), stringBytes.size());
}

void thorq_payload_serialization_unpack_1string(const std::vector<std::uint8_t> &payload, QString &string)
{
    string = QString::fromUtf8((const char*)payload.data() + 2, payload.size() - 2);
}

void thorq_payload_serialization_pack_2string(std::vector<std::uint8_t> &payload, THORQ_PAYLOAD_ID id, std::uint8_t cmd, const QString &string1, const QString &string2)
{
    QByteArray string1Bytes = string1.toUtf8();
    QByteArray string2Bytes = string2.toUtf8();

    payload.resize(4 + string1Bytes.size() + string2Bytes.size());
    payload[0] = id;
    payload[1] = cmd;
    payload[2] = string1.size();
    payload[3] = string2.size();

    memcpy(payload.data() + 4,                       string1Bytes.data(), string1Bytes.size());
    memcpy(payload.data() + 4 + string1Bytes.size(), string2Bytes.data(), string2Bytes.size());
}

void thorq_payload_serialization_pack_2string(std::vector<std::uint8_t> &payload, THORQ_PAYLOAD_ID id, std::uint8_t cmd, const QStringRef &string1, const QStringRef &string2)
{
    QByteArray string1Bytes = string1.toUtf8();
    QByteArray string2Bytes = string2.toUtf8();

    payload.resize(4 + string1Bytes.size() + string2Bytes.size());
    payload[0] = id;
    payload[1] = cmd;
    payload[2] = string1.size();
    payload[3] = string2.size();

    memcpy(payload.data() + 4,                       string1Bytes.data(), string1Bytes.size());
    memcpy(payload.data() + 4 + string1Bytes.size(), string2Bytes.data(), string2Bytes.size());
}

void thorq_payload_serialization_unpack_2string(const std::vector<std::uint8_t> &payload, QString &string1, QString &string2)
{
    const char* data = (const char*)payload.data() + 5;

    string1 = QString::fromUtf8(data, payload[2]);
    data += payload[2];

    string2 = QString::fromUtf8(data, payload[3]);
    data += payload[3];
}

void thorq_payload_serialization_pack_3string(std::vector<std::uint8_t> &payload, THORQ_PAYLOAD_ID id, std::uint8_t cmd, const QString &string1, const QString &string2, const QString &string3)
{
    QByteArray string1Bytes = string1.toUtf8();
    QByteArray string2Bytes = string2.toUtf8();
    QByteArray string3Bytes = string3.toUtf8();

    payload.resize(5 + string1Bytes.size() + string2Bytes.size() + string3Bytes.size());
    payload[0] = id;
    payload[1] = cmd;
    payload[2] = string1.size();
    payload[3] = string2.size();
    payload[4] = string3.size();

    memcpy(payload.data() + 5,                                        string1Bytes.data(), string1Bytes.size());
    memcpy(payload.data() + 5 + string1Bytes.size(),                  string2Bytes.data(), string2Bytes.size());
    memcpy(payload.data() + 5 + string1Bytes.size() + string2.size(), string3Bytes.data(), string3Bytes.size());
}

void thorq_payload_serialization_pack_3string(std::vector<std::uint8_t> &payload, THORQ_PAYLOAD_ID id, std::uint8_t cmd, const QStringRef &string1, const QStringRef &string2, const QStringRef &string3)
{
    QByteArray string1Bytes = string1.toUtf8();
    QByteArray string2Bytes = string2.toUtf8();
    QByteArray string3Bytes = string3.toUtf8();

    payload.resize(5 + string1Bytes.size() + string2Bytes.size() + string3Bytes.size());
    payload[0] = id;
    payload[1] = cmd;
    payload[2] = string1.size();
    payload[3] = string2.size();
    payload[4] = string3.size();

    memcpy(payload.data() + 5,                                        string1Bytes.data(), string1Bytes.size());
    memcpy(payload.data() + 5 + string1Bytes.size(),                  string2Bytes.data(), string2Bytes.size());
    memcpy(payload.data() + 5 + string1Bytes.size() + string2.size(), string3Bytes.data(), string3Bytes.size());
}

void thorq_payload_serialization_unpack_3string(const std::vector<std::uint8_t> &payload, QString &string1, QString &string2, QString &string3)
{
    const char* data = (const char*)payload.data() + 5;

    string1 = QString::fromUtf8(data, payload[2]);
    data += payload[2];

    string2 = QString::fromUtf8(data, payload[3]);
    data += payload[3];

    string3 = QString::fromUtf8(data, payload[4]);
    data += payload[4];
}
