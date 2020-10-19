#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <vector>
#include <cstdint>
#include <string>

#include "enums.h"

void thorq_payload_serialization_prealloc(std::vector<std::uint8_t>& payload);

enum class THORQ_TYPE : std::uint8_t
{
    NONE,

    INT8,
    INT16,
    INT32,
    INT64,

    UINT8,
    UINT16,
    UINT32,
    UINT64,

    BLOB,
    STRING,

    ENUM_MAX
};

std::uint8_t thorq_payload_serialization_get_id(const std::vector<std::uint8_t>& payload);
std::uint8_t thorq_payload_serialization_get_cmd(const std::vector<std::uint8_t>& payload);
THORQ_TYPE thorq_payload_serialization_get_type(std::vector<std::uint8_t>& payload, std::size_t& pos);

std::int8_t thorq_payload_serialization_get_int8(std::vector<std::uint8_t>& payload, std::size_t& pos);
std::int16_t thorq_payload_serialization_get_int16(std::vector<std::uint8_t>& payload, std::size_t& pos);
std::int32_t thorq_payload_serialization_get_int32(std::vector<std::uint8_t>& payload, std::size_t& pos);
std::int64_t thorq_payload_serialization_get_int64(std::vector<std::uint8_t>& payload, std::size_t& pos);

std::uint8_t thorq_payload_serialization_get_uint8(std::vector<std::uint8_t>& payload, std::size_t& pos);
std::uint16_t thorq_payload_serialization_get_uint16(std::vector<std::uint8_t>& payload, std::size_t& pos);
std::uint32_t thorq_payload_serialization_get_uint32(std::vector<std::uint8_t>& payload, std::size_t& pos);
std::uint64_t thorq_payload_serialization_get_uint64(std::vector<std::uint8_t>& payload, std::size_t& pos);

std::int8_t thorq_payload_serialization_get_int8(std::vector<std::uint8_t>& payload, std::size_t& pos);
std::int16_t thorq_payload_serialization_get_int16(std::vector<std::uint8_t>& payload, std::size_t& pos);
std::int32_t thorq_payload_serialization_get_int32(std::vector<std::uint8_t>& payload, std::size_t& pos);
std::int64_t thorq_payload_serialization_get_int64(std::vector<std::uint8_t>& payload, std::size_t& pos);

void thorq_payload_serialization_bytes_pack(std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_ID id, std::uint8_t cmd, std::initializer_list<std::uint8_t> bytes);
std::uint8_t thorq_payload_serialization_bytes_get(const std::vector<std::uint8_t>& payload, std::size_t index);
const std::uint8_t* thorq_payload_serialization_bytes_unpack(const std::vector<std::uint8_t>& payload, std::size_t& size);

void thorq_payload_serialization_pack_vector(std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_ID id, std::uint8_t cmd, const std::vector<std::uint8_t>& data);
void thorq_payload_serialization_unpack_vector(const std::vector<std::uint8_t>& payload, std::vector<std::uint8_t>& data);

void thorq_payload_serialization_pack_string(std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_ID id, std::uint8_t cmd, const std::string& string);
void thorq_payload_serialization_pack_string(std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_ID id, std::uint8_t cmd, const std::string& string1, const std::string& string2);
void thorq_payload_serialization_pack_string(std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_ID id, std::uint8_t cmd, const std::string& string1, const std::string& string2, const std::string& string3);
void thorq_payload_serialization_unpack_string(const std::vector<std::uint8_t>& payload, std::string& string);
void thorq_payload_serialization_unpack_string(const std::vector<std::uint8_t>& payload, std::string& string1, std::string& string2);
void thorq_payload_serialization_unpack_string(const std::vector<std::uint8_t>& payload, std::string& string1, std::string& string2, std::string& string3);

#endif // SERIALIZATION_H
