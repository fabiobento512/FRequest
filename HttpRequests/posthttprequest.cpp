#include "posthttprequest.h"

PostHttpRequest::PostHttpRequest
(
        QNetworkAccessManager * const manager,
        QTableWidget * const twBodyFormKeyValue,
        const QString &fullPath,
        const QString &bodyType,
        const QString &rawRequestBody,
        const QVector<UtilFRequest::HttpHeader> &requestHeaders
 )
    :HttpRequest(manager, twBodyFormKeyValue, fullPath, bodyType, rawRequestBody, requestHeaders)
{
}

QNetworkReply* PostHttpRequest::sendRequest(const QNetworkRequest &request, const QByteArray &data){
    return this->manager->post(request, data);
}
