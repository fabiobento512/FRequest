#ifndef HEADHTTPREQUEST_H
#define HEADHTTPREQUEST_H

#include "httprequest.h"

class HeadHttpRequest : public HttpRequest
{
public:
    HeadHttpRequest(
            QNetworkAccessManager * const manager,
            const QString &fullPath,
            const QVector<UtilFRequest::HttpHeader> &requestHeaders
            );
private:
    QNetworkReply* sendRequest(const QNetworkRequest &request, const QByteArray &data);
};

#endif // HEADHTTPREQUEST_H
