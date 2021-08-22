#ifndef APICLIENT_H
#define APICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>

class ApiClient : public QObject
{
    Q_OBJECT
public:
    ApiClient(QObject* parent = nullptr);
private:
    QNetworkAccessManager* m_networkAccessManager;
    QUrl m_apiUrl;
};

#endif // APICLIENT_H
