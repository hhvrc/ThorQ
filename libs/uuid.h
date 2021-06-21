#ifndef UUID_H
#define UUID_H

#if defined(_WIN32) && defined(QT_VERSION)
#define GUID_STRUCT_EXISTS
#include <QUuid>
#endif

#include <span>
#include <array>
#include <string>

namespace ThorQ {
struct Uuid
{
private:
    friend std::hash<ThorQ::Uuid>;
public:
    static ThorQ::Uuid NewUuid();
    static bool TryParse(const char* string, ThorQ::Uuid& uuidOut);
    static const ThorQ::Uuid Empty();

    Uuid() noexcept;
#ifdef GUID_STRUCT_EXISTS
    Uuid(const QUuid& other) {
        auto rfc4122 = other.toRfc4122();
        std::memcpy(m_data.data(), rfc4122.data(), 16);
    }
#endif
    Uuid(const ThorQ::Uuid& other) noexcept;
    Uuid(std::array<std::uint8_t, 16> data) noexcept;
    Uuid(std::span<const std::uint8_t, 16> data) noexcept;

    bool isEmpty() const noexcept;

    std::string toString() const;
    std::span<const std::uint8_t, 16> toBytes() const;
#ifdef GUID_STRUCT_EXISTS
    QUuid toQUuid() const {
        return QUuid(*(GUID*)m_data.data());
    }
#endif

    bool operator==(const ThorQ::Uuid& rhs) const noexcept;
    bool operator!=(const ThorQ::Uuid& rhs) const noexcept;
    bool operator< (const ThorQ::Uuid& rhs) const noexcept;
    bool operator<=(const ThorQ::Uuid& rhs) const noexcept;
    bool operator> (const ThorQ::Uuid& rhs) const noexcept;
    bool operator>=(const ThorQ::Uuid& rhs) const noexcept;

    void swap(ThorQ::Uuid& other) noexcept;

    ThorQ::Uuid operator=(const ThorQ::Uuid& other) noexcept;
private:
    std::array<std::uint8_t, 16> m_data;
};
}

namespace std {
template <>
struct hash<ThorQ::Uuid>
{
std::size_t operator()(const ThorQ::Uuid& k) const noexcept
{
    return (std::hash<std::uint64_t>()(*reinterpret_cast<const std::uint64_t*>(k.m_data.data() + 0)) ^
           (std::hash<std::uint64_t>()(*reinterpret_cast<const std::uint64_t*>(k.m_data.data() + 8)) << 1)) >> 1;
}
};
/*
template<>
void swap(ThorQ::Uuid& lhs, ThorQ::Uuid& rhs) noexcept
{
   lhs.swap(rhs);
}
*/
}

#endif // UUID_H
