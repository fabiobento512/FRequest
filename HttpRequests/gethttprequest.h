#ifndef GETHTTPREQUEST_H
#define GETHTTPREQUEST_H

#include "httprequest.h"

class GetHttpRequest : public HttpRequest
{
public:
    GetHttpRequest(
            QNetworkAccessManager * const manager,
            const QString &fullPath,
            const QVector<UtilFRequest::HttpHeader> &requestHeaders
            );
private:
    QNetworkReply* sendRequest(const QNetworkRequest &request, const QByteArray &data);
};

#endif // GETHTTPREQUEST_H
