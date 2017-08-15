#ifndef DELETEHTTPREQUEST_H
#define DELETEHTTPREQUEST_H

#include "httprequest.h"

class DeleteHttpRequest : public HttpRequest
{
public:
    DeleteHttpRequest(QNetworkAccessManager * const manager,
            const QString &fullPath,
            const QVector<UtilFRequest::HttpHeader> &requestHeaders
            );
private:
    QNetworkReply* sendRequest(const QNetworkRequest &request, const QByteArray &data);
};

#endif // DELETEHTTPREQUEST_H
