/*
 *
Copyright (C) 2017-2019  Fábio Bento (fabiobento512)

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

#ifndef HTTPREQUESTWITHMULTIPART_H
#define HTTPREQUESTWITHMULTIPART_H

#include "httprequest.h"

class HttpRequestWithMultiPart : public HttpRequest
{
public:
    HttpRequestWithMultiPart(
            QNetworkAccessManager * const manager,
            QTableWidget * const twBodyFormKeyValue,
            const QString &fullPath,
            const QString &bodyType,
            const QString &rawRequestBody,
            const QVector<UtilFRequest::HttpHeader> &requestHeaders
            );
			
	// This constructor is not used for HttpRequestWithMultiPart classes
	HttpRequestWithMultiPart
    (
            QNetworkAccessManager * const manager,
            const QString &fullPath,
            const QVector<UtilFRequest::HttpHeader> &requestHeaders
            ) = delete;
			
	virtual ~HttpRequestWithMultiPart() = default; // needed to avoid undefined behaviour https://stackoverflow.com/a/22491471/1499019
protected:
	virtual QNetworkReply* sendRequest(const QNetworkRequest &request, QHttpMultiPart &data) = 0; // abstract funtion to be filled by subclasses
	virtual QNetworkReply* sendRequest(const QNetworkRequest &request, const QByteArray &data) = 0; // abstract funtion to be filled by subclasses
private:
	QNetworkReply* sendFormRequest(QNetworkRequest &request);
};

#endif // HTTPREQUESTWITHMULTIPART_H
