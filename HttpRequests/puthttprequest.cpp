#include "puthttprequest.h"

PutHttpRequest::PutHttpRequest
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

QNetworkReply* PutHttpRequest::sendRequest(const QNetworkRequest &request, const QByteArray &data){
    return this->manager->put(request, data);
}
