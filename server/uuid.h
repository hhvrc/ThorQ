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
        uint64_t i1 = *reinterpret_cast<const std::uint64_t*>(k.m_data + 0);
        uint64_t i2 = *reinterpret_cast<const std::uint64_t*>(k.m_data + 8);
        return i1 ^ i2;
    }
  };
}

#endif // UUID_H
