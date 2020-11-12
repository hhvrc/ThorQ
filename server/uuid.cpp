#include "uuid.h"

#include <cstring>
#if defined(_WIN32) || defined(__WINDOWS__) || defined(__WIN32__)
#include <combaseapi.h>
#elif __linux__
#include <uuid/uuid.h>
#endif

ThorQ::Uuid ThorQ::Uuid::NewUuid()
{
    ThorQ::Uuid id;
#if defined(_WIN32) || defined(__WINDOWS__) || defined(__WIN32__)
    CoCreateGuid(reinterpret_cast<GUID*>(id.m_data));
#elif __linux__
    uuid_generate_random((uuid_t&)id.m_data);
#endif
    return id;
}

bool ThorQ::Uuid::TryParse(const std::string& str, ThorQ::Uuid& guidOut)
{
#if defined(_WIN32) || defined(__WINDOWS__) || defined(__WIN32__)
    return UuidFromStringA((std::uint8_t*)str.data(), (GUID*)guidOut.m_data) == RPC_S_OK;
#elif __linux__
    return uuid_parse(str.data(), (uuid_t&)guidOut.m_data) == 0;
#endif
}

const ThorQ::Uuid ThorQ::Uuid::Empty{};

ThorQ::Uuid::Uuid() noexcept
{
    memset(m_data, 0, 16);
}

ThorQ::Uuid::Uuid(const ThorQ::Uuid& other) noexcept
{
    memcpy(m_data, other.m_data, 16);
}

ThorQ::Uuid::Uuid(std::array<uint8_t, 16> data)
{
    memcpy(m_data, data.data(), 16);
}

constexpr bool ThorQ::Uuid::isEmpty() const
{
    for (uint8_t i = 0; i < 16; i++)
    {
        if (m_data[i] != 0)
        {
            return false;
        }
    }
    return true;
}


std::string ThorQ::Uuid::toString() const
{
    std::string str;
#if defined(_WIN32) || defined(__WINDOWS__) || defined(__WIN32__)
    std::uint8_t* ptr;
    if (UuidToStringA((UUID*)m_data, &ptr) != RPC_S_OK)
    {
        return str;
    }
    str.resize(36);
    memcpy(str.data(), ptr, 36);

    RpcStringFreeA(&ptr);
#elif __linux__
    str.resize(36);
    uuid_unparse_lower(m_data, str.data());
#endif

    return str;
}

std::array<uint8_t, 16> ThorQ::Uuid::toBytes() const
{
    std::array<uint8_t, 16> ret;
    memcpy(ret.data(), m_data, 16);
    return ret;
}

bool ThorQ::Uuid::operator==(const ThorQ::Uuid& rhs) const noexcept
{
    return memcmp(m_data, rhs.m_data, 16) == 0;
}
bool ThorQ::Uuid::operator!=(const ThorQ::Uuid& rhs) const noexcept
{
    return !(*this == rhs);
}
bool ThorQ::Uuid::operator<(const ThorQ::Uuid& rhs) const noexcept
{
    return memcmp(m_data, rhs.m_data, 16) < 0;
}
bool ThorQ::Uuid::operator<=(const ThorQ::Uuid& rhs) const noexcept
{
    return !(rhs < *this);
}
bool ThorQ::Uuid::operator>(const ThorQ::Uuid& rhs) const noexcept
{
    return rhs < *this;
}
bool ThorQ::Uuid::operator>=(const ThorQ::Uuid& rhs) const noexcept
{
    return !(*this < rhs);
}

ThorQ::Uuid ThorQ::Uuid::operator=(const ThorQ::Uuid& other) noexcept
{
    memcpy(m_data, other.m_data, 16);
    return *this;
}
