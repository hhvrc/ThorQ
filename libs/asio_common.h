#ifndef ASIO_COMMON_H
#define ASIO_COMMON_H

#ifdef _WIN32
#define _WIN32_WINRT 0x0A00
#endif

#define ASIO_STANDALONE

#if defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wswitch-enum"
#elif defined(_MSC_VER)
#pragma warning( push, 1 )
#endif

#include <asio/ts/internet.hpp>
#include <asio/ts/buffer.hpp>
#include <asio.hpp>

#if defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning( pop )
#endif

#endif // ASIO_COMMON_H
