/*
 *
Copyright (C) 2017  Fábio Bento (random-guy)

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

#include <QComboBox>

namespace UtilFRequest{

struct HttpHeader{
    QString name;
    QString value;
};

struct HttpFormKeyValue{
    QString key;
    QString value;
};

// Update the vector bellow always that you update this enum
enum class RequestType{
    GET_OPTION = 0,
    POST_OPTION,
    PUT_OPTION,
    DELETE_OPTION,
    PATCH_OPTION,
    HEAD_OPTION,
    TRACE_OPTION,
    OPTIONS_OPTION
};

const QVector<UtilFRequest::RequestType> possibleRequestTypes = {
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
    FORM_DATA,
    X_FORM_WWW_URLENCODED
};

QString getDocumentsFolder();
RequestType getRequestTypeByText(const QString &currentRequestText);
bool requestTypeMayHaveBody(RequestType currentRequestType);
QString getRequestTypeText(const RequestType currentRequestType);
}

#endif // UTILFREQUEST_H
