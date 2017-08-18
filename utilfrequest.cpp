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

#include "utilfrequest.h"

namespace UtilFRequest{

QString getDocumentsFolder(){

    QString result = Util::FileSystem::normalizePath(QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory));

    if(result.endsWith("/")){
        result = result.remove(result.size()-1,1);
    }

    return result;
}


RequestType getRequestTypeByText(const QString &currentRequestText){

    if(currentRequestText == "GET"){
        return RequestType::GET_OPTION;
    }
    else if(currentRequestText == "POST"){
        return RequestType::POST_OPTION;
    }
    else if(currentRequestText == "PUT"){
        return RequestType::PUT_OPTION;
    }
    else if(currentRequestText == "DELETE"){
        return RequestType::DELETE_OPTION;
    }
    else if(currentRequestText == "PATCH"){
        return RequestType::PATCH_OPTION;
    }
    else if(currentRequestText == "HEAD"){
        return RequestType::HEAD_OPTION;
    }
    else if(currentRequestText == "TRACE"){
        return RequestType::TRACE_OPTION;
    }
    else if(currentRequestText == "OPTIONS"){
        return RequestType::OPTIONS_OPTION;
    }
    else{
        QString errorMessage = "Request type unknown: '" + currentRequestText + "'. Program can't proceed.";
        Util::Dialogs::showError(errorMessage);
        LOG_FATAL << errorMessage;
        exit(1);
    }
}

QString getRequestTypeText(const RequestType currentRequestType){

    switch(currentRequestType){
    case UtilFRequest::RequestType::GET_OPTION:
        return "GET";
    case UtilFRequest::RequestType::POST_OPTION:
        return "POST";
    case UtilFRequest::RequestType::PUT_OPTION:
        return "PUT";
    case UtilFRequest::RequestType::DELETE_OPTION:
        return "DELETE";
    case UtilFRequest::RequestType::PATCH_OPTION:
        return "PATCH";
    case UtilFRequest::RequestType::HEAD_OPTION:
        return "HEAD";
    case UtilFRequest::RequestType::TRACE_OPTION:
        return "TRACE";
    case UtilFRequest::RequestType::OPTIONS_OPTION:
        return "OPTIONS";
    default:
    {
        QString errorMessage = "Invalid request type " + QString::number(static_cast<int>(currentRequestType)) + "'. Program can't proceed.";
        Util::Dialogs::showError(errorMessage);
        LOG_FATAL << errorMessage;
        exit(1);
    }
    }
}


bool requestTypeMayHaveBody(RequestType currentRequestType){

    bool mayHaveBody = false;

    switch(currentRequestType){
    case UtilFRequest::RequestType::GET_OPTION:
    case UtilFRequest::RequestType::DELETE_OPTION:
    case UtilFRequest::RequestType::HEAD_OPTION:
    case UtilFRequest::RequestType::TRACE_OPTION:
    {
        mayHaveBody = false;
        break;
    }
    case UtilFRequest::RequestType::POST_OPTION:
    case UtilFRequest::RequestType::PUT_OPTION:
    case UtilFRequest::RequestType::PATCH_OPTION:
    case UtilFRequest::RequestType::OPTIONS_OPTION:
    {
        mayHaveBody = true;
        break;
    }
    default:
    {
        QString errorMessage = "Request type unknown: '" + QString::number(static_cast<int>(currentRequestType)) + "'. Program can't proceed.";
        Util::Dialogs::showError(errorMessage);
        LOG_FATAL << errorMessage;
        exit(1);
    }
    }

    return mayHaveBody;
}

}
