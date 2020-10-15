#ifndef SYSTEMID_H
#define SYSTEMID_H

#include <cstdint>
#include <vector>
#include <string>

namespace ThorQ {
namespace SystemID_Internal {

/**
 * @brief getMacHash
 * @param mac1
 * @param mac2
 */
void getMacHash(std::uint16_t& mac1, std::uint16_t& mac2);

/**
 * @brief getVolumeHash
 * @return
 */
std::uint16_t getVolumeHash();

/**
 * @brief getCpuHash
 * @return
 */
std::uint16_t getCpuHash();

/**
 * @brief getMachineName
 * @return
 */
const char* getMachineName();
}

/**
 * @brief systemid_generate
 * @return
 */
std::vector<std::uint8_t> systemid_generate();

/**
 * @brief systemid_validate
 * @param sys_id
 * @return
 */
bool systemid_validate(const std::vector<std::uint8_t>& sys_id);

/**
 * @brief systemid_to_string
 * @param sys_id
 * @return
 */
std::string systemid_to_string(std::vector<std::uint8_t> sys_id);
}

#endif // SYSTEMID_H
