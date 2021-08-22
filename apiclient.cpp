#include "apiclient.h"

#include <QNetworkRequest>
#include <QNetworkReply>

ApiClient::ApiClient(QObject* parent)
    : QObject(parent)
    , m_networkAccessManager(new QNetworkAccessManager(this))
    , m_apiUrl("https://api.heavenvr.tech/thorq/")
{
    QNetworkRequest req(m_apiUrl.resolved(QUrl("health")));
    QNetworkReply* reply = m_networkAccessManager->get(req);
    connect(reply, &QNetworkReply::finished, [reply](){
        qDebug() << reply->readAll();
    });
}
