#ifndef DECLERATIONS_QT_H
#define DECLERATIONS_QT_H

#include <enums.h>

#include <QMetaType>

#include <cstdint>

Q_DECLARE_METATYPE(ConnectionStatus)
Q_DECLARE_METATYPE(ProcessStatus)
Q_DECLARE_METATYPE(std::uint64_t)
Q_DECLARE_METATYPE(std::uint32_t)
Q_DECLARE_METATYPE(std::uint16_t)
Q_DECLARE_METATYPE(std::uint8_t)
Q_DECLARE_METATYPE(std::int64_t)
Q_DECLARE_METATYPE(std::int32_t)
Q_DECLARE_METATYPE(std::int16_t)
Q_DECLARE_METATYPE(std::int8_t)

#endif // DECLERATIONS_QT_H
