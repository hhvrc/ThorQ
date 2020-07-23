#ifndef SYSTEMID_H
#define SYSTEMID_H

#include <string>
#include <cstdint>
#include <vector>

namespace ThorQ {
namespace SystemID_Internal {
void getMacHash(std::uint16_t& mac1, std::uint16_t& mac2);
std::uint16_t getVolumeHash();
std::uint16_t getCpuHash();
const char* getMachineName();
}

std::vector<std::uint8_t> systemid_generate();
bool systemid_validate(const std::vector<std::uint8_t>& sys_id);
std::string systemid_to_string(const std::vector<std::uint8_t>& sys_id);
}

#endif // SYSTEMID_H
