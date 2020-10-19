#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <vector>
#include <cstdint>
#include <string>

#include "enums.h"

void thorq_payload_serialization_prealloc(std::vector<std::uint8_t>& payload);

bool thorq_payload_serialization_is_valid(const std::vector<std::uint8_t>& payload);
std::uint8_t thorq_payload_serialization_get_id(const std::vector<std::uint8_t>& payload);
std::uint8_t thorq_payload_serialization_get_cmd(const std::vector<std::uint8_t>& payload);

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
