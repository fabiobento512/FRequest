#include "tracehttprequest.h"

TraceHttpRequest::TraceHttpRequest
(
        QNetworkAccessManager * const manager,
        const QString &fullPath,
        const QVector<UtilFRequest::HttpHeader> &requestHeaders
 )
    :HttpRequest(manager, fullPath, requestHeaders)
{
}

QNetworkReply* TraceHttpRequest::sendRequest(const QNetworkRequest &request, const QByteArray &){
    return sendHttpCustomRequest(request, "TRACE", QByteArray());
}
