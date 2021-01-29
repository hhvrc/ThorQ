#ifndef ASIO_COMMON_H
#define ASIO_COMMON_H

#ifdef _WIN32
#define _WIN32_WINRT 0x0A00
#endif

#define ASIO_STANDALONE

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wswitch-enum"

#include <asio/ts/internet.hpp>
#include <asio/ts/buffer.hpp>
#include <asio.hpp>

#pragma GCC diagnostic pop

#endif // ASIO_COMMON_H
