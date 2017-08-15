#ifndef POSTHTTPREQUEST_H
#define POSTHTTPREQUEST_H

#include "httprequest.h"

class PostHttpRequest : public HttpRequest
{
public:
    PostHttpRequest(
            QNetworkAccessManager * const manager,
            QTableWidget * const twBodyFormKeyValue,
            const QString &fullPath,
            const QString &bodyType,
            const QString &rawRequestBody,
            const QVector<UtilFRequest::HttpHeader> &requestHeaders
            );
private:
    QNetworkReply* sendRequest(const QNetworkRequest &request, const QByteArray &data);
};

#endif // POSTHTTPREQUEST_H
