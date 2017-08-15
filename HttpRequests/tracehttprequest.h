#ifndef TRACEHTTPREQUEST_H
#define TRACEHTTPREQUEST_H

#include "httprequest.h"

class TraceHttpRequest : public HttpRequest
{
public:
    TraceHttpRequest(
            QNetworkAccessManager * const manager,
            const QString &fullPath,
            const QVector<UtilFRequest::HttpHeader> &requestHeaders
            );
private:
    QNetworkReply* sendRequest(const QNetworkRequest &request, const QByteArray &data);
};

#endif // TRACEHTTPREQUEST_H
