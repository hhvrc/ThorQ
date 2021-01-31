#ifndef ENCODING_H
#define ENCODING_H

#include "typedefs_global.h"

#include <span>
#include <vector>
#include <memory>
#include <cstdint>

namespace ThorQ {
namespace Encoding {

bool validateEncodedData(const std::span<const std::uint8_t> data);

std::size_t calculateEncodedSize(std::size_t size, bool encrypt);
std::size_t calculateDecodedSize(const std::span<const std::uint8_t> data);

bool dataEncode(const std::span<const std::uint8_t> in, std::span<std::uint8_t> out);
bool dataEncode(const std::span<const std::uint8_t> in, std::span<std::uint8_t> out, const ThorQ::Crypto::Encryption& crypto);
bool dataDecode(const std::span<const std::uint8_t> in, std::span<std::uint8_t> out, const ThorQ::Crypto::Encryption& crypto);

}
}

#endif // ENCODING_H
