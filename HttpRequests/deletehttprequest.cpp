#include "deletehttprequest.h"

DeleteHttpRequest::DeleteHttpRequest
(
        QNetworkAccessManager * const manager,
        const QString &fullPath,
        const QVector<UtilFRequest::HttpHeader> &requestHeaders
 )
    :HttpRequest(manager, fullPath, requestHeaders)
{
}

QNetworkReply* DeleteHttpRequest::sendRequest(const QNetworkRequest &request, const QByteArray &){
    return this->manager->deleteResource(request);
}
