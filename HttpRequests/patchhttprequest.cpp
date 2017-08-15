#include "patchhttprequest.h"

PatchHttpRequest::PatchHttpRequest
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

QNetworkReply* PatchHttpRequest::sendRequest(const QNetworkRequest &request, const QByteArray &data){
    return sendHttpCustomRequest(request, "PATCH", data);
}
