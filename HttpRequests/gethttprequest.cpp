#include "gethttprequest.h"

GetHttpRequest::GetHttpRequest
(
        QNetworkAccessManager * const manager,
        const QString &fullPath,
        const QVector<UtilFRequest::HttpHeader> &requestHeaders
 )
    :HttpRequest(manager, fullPath, requestHeaders)
{
}

QNetworkReply* GetHttpRequest::sendRequest(const QNetworkRequest &request, const QByteArray &){
    return this->manager->get(request);
}
