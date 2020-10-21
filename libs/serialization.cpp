#include "serialization.h"

#include <memory>
#include <cstring>

#include "constants.h"
#include "thorq_message.h"

void thorq_payload_serialization_prealloc(std::vector<std::uint8_t>& payload)
{
    payload.reserve(THORQ_PAYLOAD_LEN_MAX);
}

std::uint8_t thorq_payload_serialization_get_id(const std::vector<std::uint8_t> &payload)
{
    return payload[0];
}

std::uint8_t thorq_payload_serialization_get_cmd(const std::vector<std::uint8_t> &payload)
{
    return payload[1];
}

THORQ_TYPE thorq_payload_serialization_get_type(std::vector<uint8_t>& payload, std::size_t& pos)
{
    if (pos < payload.size() && payload[pos] < (std::uint8_t)THORQ_TYPE::ENUM_MAX)
    {
        return (THORQ_TYPE)payload[pos];
    }

    return THORQ_TYPE::NONE;
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

void thorq_payload_serialization_pack_bytearray(std::vector<std::uint8_t> &payload, THORQ_PAYLOAD_ID id, std::uint8_t cmd, const std::vector<std::uint8_t> &data)
{
    payload.resize(2 + data.size());
    payload[0] = id;
    payload[1] = cmd;

    memcpy(payload.data() + 2, data.data(), data.size());
}

void thorq_payload_serialization_unpack_bytearray(const std::vector<std::uint8_t> &payload, std::vector<std::uint8_t> &data)
{
    data.resize(payload.size() - 2);
    memcpy(data.data(), payload.data() + 2, payload.size() - 2);
}

void thorq_payload_serialization_pack_string(std::vector<std::uint8_t> &payload, THORQ_PAYLOAD_ID id, std::uint8_t cmd, const std::string &string)
{
    payload.resize(2 + string.size());
    payload[0] = id;
    payload[1] = cmd;

    memcpy(payload.data() + 2, string.data(), string.size());
}

void thorq_payload_serialization_unpack_string(const std::vector<std::uint8_t> &payload, std::string &string)
{
    string = std::string((const char*)payload.data() + 2, payload.size() - 2);
}

void thorq_payload_serialization_pack_string(std::vector<std::uint8_t> &payload, THORQ_PAYLOAD_ID id, std::uint8_t cmd, const std::string &string1, const std::string &string2)
{
    payload.resize(4 + string1.size() + string2.size());
    payload[0] = id;
    payload[1] = cmd;
    payload[2] = string1.size();
    payload[3] = string2.size();

    memcpy(payload.data() + 4,                  string1.data(), string1.size());
    memcpy(payload.data() + 4 + string1.size(), string2.data(), string2.size());
}

void thorq_payload_serialization_unpack_string(const std::vector<std::uint8_t> &payload, std::string &string1, std::string &string2)
{
    const char* data = (const char*)payload.data() + 5;

    string1 = std::string(data, payload[2]);
    data += payload[2];

    string2 = std::string(data, payload[3]);
    data += payload[3];
}

void thorq_payload_serialization_pack_string(std::vector<std::uint8_t> &payload, THORQ_PAYLOAD_ID id, std::uint8_t cmd, const std::string &string1, const std::string &string2, const std::string &string3)
{
    payload.resize(5 + string1.size() + string2.size() + string3.size());
    payload[0] = id;
    payload[1] = cmd;
    payload[2] = string1.size();
    payload[3] = string2.size();
    payload[4] = string3.size();

    memcpy(payload.data() + 5,                                   string1.data(), string1.size());
    memcpy(payload.data() + 5 + string1.size(),                  string2.data(), string2.size());
    memcpy(payload.data() + 5 + string1.size() + string2.size(), string3.data(), string3.size());
}

void thorq_payload_serialization_unpack_string(const std::vector<std::uint8_t> &payload, std::string &string1, std::string &string2, std::string &string3)
{
    const char* data = (const char*)payload.data() + 5;

    string1 = std::string(data, payload[2]);
    data += payload[2];

    string2 = std::string(data, payload[3]);
    data += payload[3];

    string3 = std::string(data, payload[4]);
    data += payload[4];
}
