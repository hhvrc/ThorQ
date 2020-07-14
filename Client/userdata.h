#ifndef USERDATA_H
#define USERDATA_H

#include <QString>

struct UserData
{
public:
    QString username = "";
    bool inSession = false;
    bool hasCollar = false;
};

#endif // USERDATA_H
