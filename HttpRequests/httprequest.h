#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QVector>
#include <QUrlQuery>

#include "utilfrequest.h"
#include <QBuffer>

class HttpRequest
{
public:
    HttpRequest
    (
            QNetworkAccessManager * const manager,
            QTableWidget * const twBodyFormKeyValue,
            const QString &fullPath,
            const QString &bodyType,
            const QString &rawRequestBody,
            const QVector<UtilFRequest::HttpHeader> &requestHeaders
            );
    HttpRequest
    (
            QNetworkAccessManager * const manager,
            const QString &fullPath,
            const QVector<UtilFRequest::HttpHeader> &requestHeaders
            );
    virtual ~HttpRequest(); // needed to avoid undefined behaviour https://stackoverflow.com/a/22491471/1499019

    QNetworkReply* processRequest();
private:
    const QString fullPath;
    const QVector<UtilFRequest::HttpHeader> requestHeaders;
    const QString bodyType;
    const QString rawRequestBody;
protected:
    QNetworkAccessManager * const manager;
    QTableWidget * const twBodyFormKeyValue;
protected:
    virtual QNetworkReply* sendRequest(const QNetworkRequest &request, const QByteArray &data) = 0; // abstract funtion to be filled by subclasses
    QNetworkReply* sendHttpCustomRequest(const QNetworkRequest &request, const QString &verb, const QByteArray &data);
private:
    QNetworkReply* sendFormRequest(QNetworkRequest &request);
};

#endif // HTTPREQUEST_H
