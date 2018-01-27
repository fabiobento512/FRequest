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

#include "httprequest.h"

HttpRequest::HttpRequest
(
        QNetworkAccessManager * const manager,
        QTableWidget * const twBodyFormKeyValue,
        const QString &fullPath,
        const QString &bodyType,
        const QString &rawRequestBody,
        const QVector<UtilFRequest::HttpHeader> &requestHeaders
        )
    :fullPath(fullPath),
      requestHeaders(requestHeaders), rawRequestBody(rawRequestBody), 
	  bodyType(bodyType), manager(manager), twBodyFormKeyValue(twBodyFormKeyValue)
{

}

HttpRequest::HttpRequest
(
        QNetworkAccessManager * const manager,
        const QString &fullPath,
        const QVector<UtilFRequest::HttpHeader> &requestHeaders
        )
    :fullPath(fullPath),
      requestHeaders(requestHeaders),
      manager(manager),
      twBodyFormKeyValue(nullptr)
{
	
}

QNetworkReply* HttpRequest::processRequest(){

    QNetworkRequest request(QUrl(this->fullPath));

    for(const UtilFRequest::HttpHeader &currentHeader : this->requestHeaders){
		// If multipart, don't add the multipart/form-data header, it is added automatically by HttpRequestWithMultiPart subclass
		if(!(currentHeader.name == "Content-type" && currentHeader.value == "multipart/form-data")){
			request.setRawHeader(currentHeader.name.toUtf8(), currentHeader.value.toUtf8());
		}
    }
	
    if(!bodyType.isEmpty()){
        if(this->bodyType == "raw"){
            return sendRequest(request, this->rawRequestBody.toUtf8());
        }
        else if(this->bodyType == "form-data"){
            return sendFormRequest(request);
        }
        else if(this->bodyType == "x-form-www-urlencoded"){
            return sendFormRequest(request);
        }
        else{
            QString errorMessage = "Body type unknown: '" + this->bodyType + "'. Application can't proceed.";
            Util::Dialogs::showError(errorMessage);
            LOG_FATAL << errorMessage;
            exit(1);
        }
    }
    else{ // Get, delete etc which doesn't have a "body"
        return sendRequest(request, QString().toUtf8());
    }
}

QNetworkReply* HttpRequest::sendFormRequest(QNetworkRequest &request){

	QUrlQuery params;

    for(int i = 0; i < this->twBodyFormKeyValue->rowCount(); i++){
        params.addQueryItem(this->twBodyFormKeyValue->item(i,0)->text(), this->twBodyFormKeyValue->item(i,1)->text());
    }

    return sendRequest(request, params.toString(QUrl::FullyEncoded).toUtf8());

}

QNetworkReply* HttpRequest::sendHttpCustomRequest(const QNetworkRequest &request, const QString &verb, const QByteArray &data){
	
	// Based from here:
    // https://stackoverflow.com/a/34065736/1499019
    QBuffer *buffer=new QBuffer();
    if(!data.isNull() && !data.isEmpty()){
        buffer->open((QBuffer::ReadWrite));
        buffer->write(data);
        buffer->seek(0);
    }
	
    return this->manager->sendCustomRequest(request, verb.toUtf8(), buffer);
}

QNetworkReply* HttpRequest::sendHttpCustomRequest(const QNetworkRequest &request, const QString &verb, QHttpMultiPart &data){
    return this->manager->sendCustomRequest(request, verb.toUtf8(), &data);
}
