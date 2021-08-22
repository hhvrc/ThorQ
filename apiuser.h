#ifndef APIUSER_H
#define APIUSER_H

#include <QObject>
#include <QString>
#include <QImage>

class ApiUser : public QObject
{
    Q_OBJECT
public:
    ApiUser(QObject* parent = nullptr);
private:
    QString m_id;
    QString m_username;
    QImage  m_profilePicture;
};

#endif // APIUSER_H
