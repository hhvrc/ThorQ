#ifndef UUID_H
#define UUID_H

#include <array>
#include <string>

namespace ThorQ {
class Uuid
{
    friend std::hash<ThorQ::Uuid>;
public:
    static ThorQ::Uuid NewUuid();
    static bool TryParse(const std::string& str, ThorQ::Uuid& guidOut);
    static const ThorQ::Uuid Empty;

    Uuid() noexcept;
    Uuid(const Uuid& other) noexcept;
    Uuid(std::array<std::uint8_t, 16> data);

    constexpr bool isEmpty() const;

    std::string toString() const;
    std::array<std::uint8_t, 16> toBytes() const;

    bool operator==(const ThorQ::Uuid& rhs) const noexcept;
    bool operator!=(const ThorQ::Uuid& rhs) const noexcept;
    bool operator< (const ThorQ::Uuid& rhs) const noexcept;
    bool operator<=(const ThorQ::Uuid& rhs) const noexcept;
    bool operator> (const ThorQ::Uuid& rhs) const noexcept;
    bool operator>=(const ThorQ::Uuid& rhs) const noexcept;

    ThorQ::Uuid operator=(const ThorQ::Uuid& other) noexcept;
private:
    std::uint8_t m_data[16];
};
}

namespace std {

  template <>
  struct hash<ThorQ::Uuid>
  {
    std::size_t operator()(const ThorQ::Uuid& k) const
    {
        return (std::hash<std::uint64_t>()(*reinterpret_cast<const std::uint64_t*>(k.m_data + 0)) ^
               (std::hash<std::uint64_t>()(*reinterpret_cast<const std::uint64_t*>(k.m_data + 8)) << 1)) >> 1;
    }
  };
}

#endif // UUID_H
