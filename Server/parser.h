#ifndef PARSER_H
#define PARSER_H

#include <QObject>

#include <typedefs_global.h>

namespace ThorQ {
class Parser : public QObject
{
    Q_OBJECT
public:
    Parser();
public slots:
    void handleEvent(const ENetEvent& event);
};
}

#endif // PARSER_H
