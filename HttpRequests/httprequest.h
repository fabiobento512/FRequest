/*
 *
Copyright (C) 2017-2018  Fábio Bento (random-guy)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QHttpMultiPart>
#include <QVector>
#include <QUrlQuery>
#include <QNetworkReply>

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
    virtual ~HttpRequest() = default; // needed to avoid undefined behaviour https://stackoverflow.com/a/22491471/1499019

    QNetworkReply* processRequest();
private:
    const QString fullPath;
    const QVector<UtilFRequest::HttpHeader> requestHeaders;
    const QString rawRequestBody;
protected:
	const QString bodyType;
    QNetworkAccessManager * const manager;
    QTableWidget * const twBodyFormKeyValue;
protected:
    virtual QNetworkReply* sendRequest(const QNetworkRequest &request, const QByteArray &data) = 0; // abstract funtion to be filled by subclasses
    virtual QNetworkReply* sendFormRequest(QNetworkRequest &request);
    QNetworkReply* sendHttpCustomRequest(const QNetworkRequest &request, const QString &verb, const QByteArray &data);
    QNetworkReply* sendHttpCustomRequest(const QNetworkRequest &request, const QString &verb, QHttpMultiPart &data);
};

#endif // HTTPREQUEST_H
