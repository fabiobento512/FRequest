#include "optionshttprequest.h"

OptionsHttpRequest::OptionsHttpRequest
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

QNetworkReply* OptionsHttpRequest::sendRequest(const QNetworkRequest &request, const QByteArray &data){
    return sendHttpCustomRequest(request, "OPTIONS", data);
}
