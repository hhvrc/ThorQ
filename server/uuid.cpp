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
    CoCreateGuid(reinterpret_cast<GUID*>(id.m_data.data()));
#elif __linux__
    uuid_generate_random(id.m_data.data());
#endif
    return id;
}

bool ThorQ::Uuid::TryParse(const std::string& str, ThorQ::Uuid& guidOut)
{
#if defined(_WIN32) || defined(__WINDOWS__) || defined(__WIN32__)
    return UuidFromStringA((std::uint8_t*)str.data(), (GUID*)guidOut.m_data.data()) == RPC_S_OK;
#elif __linux__
    return uuid_parse(str.data(), guidOut.m_data.data()) == 0;
#endif
}

const ThorQ::Uuid ThorQ::Uuid::Empty()
{
    return ThorQ::Uuid();
}

ThorQ::Uuid::Uuid() noexcept
{
    memset(m_data.data(), 0, 16);
}

ThorQ::Uuid::Uuid(const ThorQ::Uuid& other) noexcept
{
    std::copy(other.m_data.begin(), other.m_data.end(), m_data.begin());
}

ThorQ::Uuid::Uuid(std::array<uint8_t, 16> data) noexcept
{
    std::copy(m_data.begin(), m_data.end(), data.begin());
}

std::array<std::uint8_t, 16> empty{0};
bool ThorQ::Uuid::isEmpty() const noexcept
{
    return m_data == empty;
}

std::string ThorQ::Uuid::toString() const
{
    std::string str;
#if defined(_WIN32) || defined(__WINDOWS__) || defined(__WIN32__)
    std::uint8_t* ptr;
    if (UuidToStringA((UUID*)m_data.data(), &ptr) != RPC_S_OK)
    {
        return str;
    }
    str.resize(36);
    memcpy(str.data(), ptr, 36);

    RpcStringFreeA(&ptr);
#elif __linux__
    str.resize(36);
    uuid_unparse_lower(m_data.data(), str.data());
#endif

    return str;
}

std::array<uint8_t, 16> ThorQ::Uuid::toBytes() const
{
    std::array<uint8_t, 16> ret;
    std::copy(m_data.begin(), m_data.end(), ret.begin());
    return ret;
}

bool ThorQ::Uuid::operator==(const ThorQ::Uuid& rhs) const noexcept
{
    return m_data == rhs.m_data;
}
bool ThorQ::Uuid::operator!=(const ThorQ::Uuid& rhs) const noexcept
{
    return !(*this == rhs);
}
bool ThorQ::Uuid::operator<(const ThorQ::Uuid& rhs) const noexcept
{
    return m_data < rhs.m_data;
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

void ThorQ::Uuid::swap(ThorQ::Uuid& other) noexcept
{
    m_data.swap(other.m_data);
}

ThorQ::Uuid ThorQ::Uuid::operator=(const ThorQ::Uuid& other) noexcept
{
    std::copy(other.m_data.begin(), other.m_data.end(), m_data.begin());
    return *this;
}
