#include "hashing.h"

#include <array>
#include <atomic>
#include <mutex>

std::mutex init_mtx;
bool init_done;

std::array<std::uint32_t, 256> crcTable;

constexpr std::uint32_t reflect_crc(int val, int bits)
{
    std::uint32_t result = 0;

    for (std::uint8_t bit = 0; bit < bits; bit++)
    {
        if (val & 1)
        {
            result |= 1 << (bits - 1 - bit);
        }

        val >>= 1;
    }

    return result;
}

void initialize_crc32()
{
    std::scoped_lock l(init_mtx);

    if (!init_done)
    {
        for (int byte = 0; byte < 256; ++byte)
        {
            std::uint32_t crc = reflect_crc(byte, 8) << 24;
            int offset;

            for (offset = 0; offset < 8; ++offset)
            {
                if (crc & 0x80000000)
                {
                    crc = (crc << 1) ^ 0x04c11db7;
                }
                else
                {
                    crc <<= 1;
                }
            }

            crcTable[byte] = reflect_crc(crc, 32);
        }
    }

    init_done = true;
}

std::uint32_t ThorQ::Hashing::Crc32(const std::uint8_t* data, std::size_t size)
{
    std::uint32_t crc = 0xFFFFFFFF;

    if (!init_done) { initialize_crc32(); }


    for (const std::uint8_t* end = data + size; data != end; data++)
    {
        crc = (crc >> 8) ^ crcTable[(crc & 0xFF) ^ *data];
    }

    return ~crc;
}
