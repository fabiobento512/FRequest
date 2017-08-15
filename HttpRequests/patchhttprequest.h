#ifndef PATCHHTTPREQUEST_H
#define PATCHHTTPREQUEST_H

#include "httprequest.h"

class PatchHttpRequest : public HttpRequest
{
public:
    PatchHttpRequest(
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

#endif // PATCHHTTPREQUEST_H
