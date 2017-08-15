#ifndef OPTIONSHTTPREQUEST_H
#define OPTIONSHTTPREQUEST_H

#include "httprequest.h"

class OptionsHttpRequest : public HttpRequest
{
public:
    OptionsHttpRequest(
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

#endif // OPTIONSHTTPREQUEST_H
