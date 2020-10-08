#ifndef STATISTICS_H
#define STATISTICS_H

#include <atomic>
#include <cstdint>

std::atomic<std::uint64_t> g_totalSentData;
std::atomic<std::uint64_t> g_totalSentPackets;

std::atomic<std::uint64_t> g_totalReceivedData;
std::atomic<std::uint64_t> g_totalReceivedPackets;

#endif // STATISTICS_H
