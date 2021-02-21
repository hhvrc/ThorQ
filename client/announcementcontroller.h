#ifndef ANNOUNCEMENTHANDLER_H
#define ANNOUNCEMENTHANDLER_H

#include <cryptography/hashing.h>

#include <QObject>

#include <functional>
#include <span>
#include <string>
#include <cstdint>

class AnnouncementHandler : public QObject
{
    Q_OBJECT
public:
    AnnouncementHandler(std::function<void(const std::span<std::uint8_t>&, bool)> encodeAndSend, QObject* parent = nullptr);

public slots:
    void ParseMessage(const void* message);
private:

};

#endif // ANNOUNCEMENTHANDLER_H
