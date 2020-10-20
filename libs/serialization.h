#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <vector>
#include <cstdint>
#include <string>

#include "enums.h"

void thorq_payload_serialization_prealloc(std::vector<std::uint8_t>& payload);


template <class ContainerType>
concept Container = requires(ContainerType container)
{
    requires std::regular<ContainerType>;
    requires std::swappable<ContainerType>;
    requires std::destructible<typename ContainerType::value_type>;
    requires std::same_as<typename ContainerType::reference, typename ContainerType::value_type &>;
    requires std::same_as<typename ContainerType::const_reference, const typename ContainerType::value_type &>;
    requires std::forward_iterator<typename ContainerType::iterator>;
    requires std::forward_iterator<typename ContainerType::const_iterator>;
    requires std::signed_integral<typename ContainerType::difference_type>;
    requires std::unsigned_integral<typename ContainerType::size_type>;
//  { container.begin()    } -> typename ContainerType::iterator;
//  { container.end()      } -> typename ContainerType::iterator;
//  { container.begin()    } -> typename ContainerType::const_iterator;
//  { container.end()      } -> typename ContainerType::const_iterator;
//  { container.cbegin()   } -> typename ContainerType::const_iterator;
//  { container.cend()     } -> typename ContainerType::const_iterator;
    { container.size()     } -> std::same_as<std::size_t>;
    { container.max_size() } -> std::same_as<std::size_t>;
    { container.empty()    } -> std::same_as<bool>;
};
std::size_t get_size_needed(const Container auto& value)
{
    return std::size(value) * sizeof(decltype(*value.begin()));
}

template<typename A>
requires std::is_fundamental<A>;
std::size_t get_size_needed(const A&)
{
    return sizeof(A);
}

template<typename A, typename... Args>
inline void serialize(const std::vector<std::uint8_t>& dataIn, A arg1, Args... args)
{
    serialize(dataIn, arg1);
    serialize(dataIn, args...);
}
template<typename A, typename... Args>
inline void deserialize(std::vector<std::uint8_t>& dataOut, A arg1, Args... args)
{
    serialize(dataOut, arg1);
    serialize(dataOut, args...);
}

template<typename... Args>
inline void reserve_and_serialize(std::vector<std::uint8_t>& dataOut, Args... args)
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
