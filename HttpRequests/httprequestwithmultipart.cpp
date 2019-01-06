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

#include "httprequestwithmultipart.h"

HttpRequestWithMultiPart::HttpRequestWithMultiPart
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

QNetworkReply* HttpRequestWithMultiPart::sendFormRequest(QNetworkRequest &request){

	// If not form data call default handler
	if(this->bodyType != "form-data"){
		return HttpRequest::sendFormRequest(request);
	}
	
	// Process multi part (form data) request
	QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
	
	for(int i = 0; i < this->twBodyFormKeyValue->rowCount(); i++){
		
		// TODO create enums to these columns, so we don't have to put the index directly
		const QString &currKey = this->twBodyFormKeyValue->item(i,0)->text();
		const QString &currValue = this->twBodyFormKeyValue->item(i,1)->text();
		const UtilFRequest::FormKeyValueType currFormKeyValueType = UtilFRequest::getFormKeyTypeByString(this->twBodyFormKeyValue->item(i,2)->text());
		
		 switch(currFormKeyValueType){
			case UtilFRequest::FormKeyValueType::TEXT:
			{
				QHttpPart textPart;
				textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"" + currKey + "\""));
				textPart.setBody(currValue.toUtf8());

				multiPart->append(textPart);
				
				break;
			}
			case UtilFRequest::FormKeyValueType::FILE:
			{
				QHttpPart filePart;
                filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(UtilFRequest::mimeDatabase.mimeTypeForFile(currValue, QMimeDatabase::MatchMode::MatchExtension).name()));
				filePart.setHeader(
				QNetworkRequest::ContentDispositionHeader, 
				QVariant("form-data; name=\"" + Util::FileSystem::cutNameWithoutBackSlash(currKey) + "\"; filename=\"" + Util::FileSystem::cutNameWithoutBackSlash(currValue) + "\"")
				);
				
				QFile *currFile = new QFile(currValue);
				
				currFile->open(QIODevice::ReadOnly);
				filePart.setBodyDevice(currFile);
				currFile->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart
				
				multiPart->append(filePart);
				break;
			}
			default:
			{
				QString errorMessage = "Invalid form key type " + QString::number(static_cast<int>(currFormKeyValueType)) + "'. Program can't proceed.";
				Util::Dialogs::showError(errorMessage);
				LOG_FATAL << errorMessage;
				exit(1);
			}
		}

   }
   
   QNetworkReply *reply = sendRequest(request, *multiPart);
   
   multiPart->setParent(reply);
   
   return reply;
}
