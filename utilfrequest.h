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

#ifndef UTILFREQUEST_H
#define UTILFREQUEST_H

#include "util.h"

// PLog library (log library)
// https://github.com/SergiusTheBest/plog
#include <plog/Log.h>
#include <plog/Converters/NativeEOLConverter.h>
#include <pugixml/pugixml.hpp>
#include <cpp17optional/optional.hpp>

#include <QComboBox>
#include <QDateTime>
#include <QMimeDatabase>
#include <QJsonDocument>

#include "Authentications/frequestauthentication.h"
#include "utilglobalvars.h"

namespace UtilFRequest{

struct HttpHeader{
    QString name;
    QString value;
};

// Update the vector bellow always that you update this enum
enum class RequestType{
    GET_OPTION = 0,
    POST_OPTION = 1,
    PUT_OPTION = 2,
    DELETE_OPTION = 3,
    PATCH_OPTION = 4,
    HEAD_OPTION = 5,
    TRACE_OPTION = 6,
    OPTIONS_OPTION = 7
};

const QVector<RequestType> possibleRequestTypes = {
    UtilFRequest::RequestType::GET_OPTION,
    UtilFRequest::RequestType::POST_OPTION,
    UtilFRequest::RequestType::PUT_OPTION,
    UtilFRequest::RequestType::DELETE_OPTION,
    UtilFRequest::RequestType::PATCH_OPTION,
    UtilFRequest::RequestType::HEAD_OPTION,
    UtilFRequest::RequestType::TRACE_OPTION,
    UtilFRequest::RequestType::OPTIONS_OPTION
};

enum class BodyType{
    RAW = 0,
    FORM_DATA = 1,
    X_FORM_WWW_URLENCODED = 2
};

enum class FormKeyValueType{
    TEXT = 0,
    FILE = 1
};

enum class SerializationFormatType{
	UNKNOWN = 0,
	JSON = 1,
	XML = 2
};

struct HttpFormKeyValueType{
	
	QString key;
    QString value;
	FormKeyValueType type;
	
	HttpFormKeyValueType(){}
	
	HttpFormKeyValueType(const QString &key, const QString &value, const FormKeyValueType type){
		this->key = key;
		this->value = value;
		this->type = type;
	}
	
};

struct RequestInfo{
	bool bOverridesMainUrl = false;
	QString overrideMainUrl;
	QString name;
	QString path;
	RequestType requestType = RequestType::GET_OPTION;
	BodyType bodyType = BodyType::RAW;
	QString body;
	QVector<HttpFormKeyValueType> bodyForm;
	QVector<HttpHeader> headers;
	bool bDownloadResponseAsFile = false;
	QString uuid;
	unsigned long long int order = 0;
};

static QMimeDatabase mimeDatabase;

QString getDocumentsFolder();
bool requestTypeMayHaveBody(RequestType currentRequestType);
RequestType getRequestTypeByString(const QString &currentRequestText);
QString getRequestTypeString(const RequestType currentRequestType);

FormKeyValueType getFormKeyTypeByString(const QString &currentFormKeyValueString);
QString getFormKeyTypeString(const FormKeyValueType currentFormKeyValueType);

BodyType getBodyTypeByString(const QString &currentBodyTypeText);
QString getBodyTypeString(const BodyType currentBodyType);

QString getDateTimeFormatForFilename(const QDateTime &currentDateTime);

void addRequestFormBodyRow(QTableWidget * const myTable, const QString &key, const QString &value, const UtilFRequest::FormKeyValueType type);

// Return original content in case of error
QString getStringFormattedForSerializationType(const QString &content, const SerializationFormatType serializationType);
SerializationFormatType getSerializationFormatTypeForString(const QString &content);

// Simply ofuscation just to not store password string as plain text in project / configuration files
// (it does not protect the password, just obfuscates so can't be read directly)
//  Applies a simple xor with the given salt
QByteArray simpleStringObfuscationDeobfuscation(const QString& ofuscationSalt, const QString &input);

// Replaces the textToReplace string with the actual username and password given (uses the FRequest Auth Placeholders for the replace)
QString replaceFRequestAuthenticationPlaceholders(const QString &textToReplace, const QString &username, const QString &password);

}

#endif // UTILFREQUEST_H
