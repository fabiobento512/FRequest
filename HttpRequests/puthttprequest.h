#ifndef PUTHTTPREQUEST_H
#define PUTHTTPREQUEST_H

#include "httprequest.h"

class PutHttpRequest : public HttpRequest
{
public:
    PutHttpRequest(
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

#endif // PUTHTTPREQUEST_H
