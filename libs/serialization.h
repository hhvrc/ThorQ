#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <vector>
#include <cstdint>
#include <string>
#include <ranges>
#include <numeric>

#include "enums.h"

void thorq_payload_serialization_prealloc(std::vector<std::uint8_t>& payload);

#ifdef _MSC_VER // /std:c++latest /O2
#include <type_traits>
template <typename T>
constexpr std::size_t get_size_needed_for_type(const T& value)
{
    if constexpr (std::is_arithmetic_v<T>)
    {
        return sizeof(value);
    }
    else if constexpr (std::is_class_v<T> || std::is_union_v<T>)
    {
        return std::accumulate(value.cbegin(), value.cend(), std::size_t{ 0 }, [](std::size_t acc, const auto& v)
        {
            return acc + get_size_needed_for_type(v);
        });
    }
    else
    {
        static_assert(false, "Unsupported type for serialization!");
    }
}
#else           // -std=c++2a -O3
template <typename T>
    requires std::integral<T> || std::floating_point<T>
constexpr std::size_t get_size_needed_for_type(const T& value)
{
    return sizeof(value);
}
constexpr std::size_t get_size_needed_for_type(const std::ranges::range auto& value)
{
    return std::accumulate(value.cbegin(), value.cend(), std::size_t{ 0 }, [](std::size_t acc, const auto& v)
    {
        return acc +                        // Accumulated size
               sizeof(std::uint32_t) +      // Space for size descriptor
               get_size_needed_for_type(v); // Space for data
    });
}
#endif

template <typename... Args>
constexpr std::size_t get_size_needed(const Args&... args) {
  return (... + get_size_needed_for_type(args));
}

template <typename T>
    requires std::integral<T> || std::floating_point<T>
inline void serialize(std::vector<std::uint8_t>& dataIn, const T& value)
{
    dataIn.resize(dataIn.size() + sizeof(value));
    memcpy(dataIn.data() + dataIn.size() - sizeof(value), &value, sizeof(value));
}

template<typename A, typename... Args>
inline void serialize(std::vector<std::uint8_t>& dataIn, const A& arg1, const Args&... args)
{
    serialize(dataIn, arg1);
    serialize(dataIn, args...);
}
template<typename A, typename... Args>
inline void deserialize(std::vector<std::uint8_t>& dataOut, const A& arg1, const Args&... args)
{
    serialize(dataOut, arg1);
    serialize(dataOut, args...);
}

template<typename... Args>
inline void reserve_and_serialize(std::vector<std::uint8_t>& dataOut, const Args&... args)
{
  dataOut.reserve(get_size_needed(args...));
  serialize(dataOut, args...);
}

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
