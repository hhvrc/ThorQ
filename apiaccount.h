#ifndef APIACCOUNT_H
#define APIACCOUNT_H

#include "apiuser.h"

class ApiAccount : public ApiUser
{
    Q_OBJECT
public:
    ApiAccount(QObject* parent = nullptr);
private:
    QString m_email;
};

#endif // APIACCOUNT_H
