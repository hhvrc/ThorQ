#ifndef USERDATA_H
#define USERDATA_H

#include <QString>

struct UserData
{
public:
    QString username = "";
    bool hasCollar = false;
    bool inSession = false;
};

#endif // USERDATA_H
