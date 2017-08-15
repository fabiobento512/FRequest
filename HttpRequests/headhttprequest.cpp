#include "headhttprequest.h"

HeadHttpRequest::HeadHttpRequest
(
        QNetworkAccessManager * const manager,
        const QString &fullPath,
        const QVector<UtilFRequest::HttpHeader> &requestHeaders
 )
    :HttpRequest(manager, fullPath, requestHeaders)
{
}

QNetworkReply* HeadHttpRequest::sendRequest(const QNetworkRequest &request, const QByteArray &){
    return this->manager->head(request);
}
