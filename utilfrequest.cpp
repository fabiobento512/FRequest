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

#include "utilfrequest.h"

namespace UtilFRequest{

QString getDocumentsFolder(){

    QString result = Util::FileSystem::normalizePath(QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory));

    if(result.endsWith("/")){
        result = result.remove(result.size()-1,1);
    }

    return result;
}


RequestType getRequestTypeByString(const QString &currentRequestText){

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

QString getRequestTypeString(const RequestType currentRequestType){

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
    {
        mayHaveBody = false;
        break;
    }
    case UtilFRequest::RequestType::POST_OPTION:
    case UtilFRequest::RequestType::PUT_OPTION:
    case UtilFRequest::RequestType::PATCH_OPTION:
    case UtilFRequest::RequestType::OPTIONS_OPTION:
    case UtilFRequest::RequestType::TRACE_OPTION:
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

QString getDateTimeFormatForFilename(const QDateTime &currentDateTime){
    return currentDateTime.toString("yyyy-MM-dd_hh-mm-ss");
}

FormKeyValueType getFormKeyTypeByString(const QString &currentFormKeyValueString){

    if(currentFormKeyValueString == "Text"){
        return FormKeyValueType::TEXT;
    }
    else if(currentFormKeyValueString == "File"){
        return FormKeyValueType::FILE;
    }
    else{
        QString errorMessage = "FormKeyValue type unknown: '" + currentFormKeyValueString + "'. Program can't proceed.";
        Util::Dialogs::showError(errorMessage);
        LOG_FATAL << errorMessage;
        exit(1);
    }
}

QString getFormKeyTypeString(const FormKeyValueType currentFormKeyValueType){

    switch(currentFormKeyValueType){
    case UtilFRequest::FormKeyValueType::TEXT:
        return "Text";
    case UtilFRequest::FormKeyValueType::FILE:
        return "File";
    default:
    {
        QString errorMessage = "Invalid form key type " + QString::number(static_cast<int>(currentFormKeyValueType)) + "'. Program can't proceed.";
        Util::Dialogs::showError(errorMessage);
        LOG_FATAL << errorMessage;
        exit(1);
    }
    }
}

BodyType getBodyTypeByString(const QString &currentBodyTypeText){
    if(currentBodyTypeText == "raw"){
        return BodyType::RAW;
    }
    else if(currentBodyTypeText == "form-data"){
        return BodyType::FORM_DATA;
    }
    else if(currentBodyTypeText == "x-form-www-urlencoded"){
        return BodyType::X_FORM_WWW_URLENCODED;
    }
    else{
        QString errorMessage = "BodyType type unknown: '" + currentBodyTypeText + "'. Program can't proceed.";
        Util::Dialogs::showError(errorMessage);
        LOG_FATAL << errorMessage;
        exit(1);
    }
}

QString getBodyTypeString(const BodyType currentBodyType){
    switch(currentBodyType){
    case UtilFRequest::BodyType::RAW:
    {
        return "raw";
    }
    case UtilFRequest::BodyType::FORM_DATA:
    {
        return "form-data";
    }
    case UtilFRequest::BodyType::X_FORM_WWW_URLENCODED:
    {
        return "x-form-www-urlencoded";
    }
    default:
    {
        QString errorMessage = "Invalid body type " + QString::number(static_cast<int>(currentBodyType)) + "'. Program can't proceed.";
        Util::Dialogs::showError(errorMessage);
        LOG_FATAL << errorMessage;
        exit(1);
    }
    }
}

void addRequestFormBodyRow(QTableWidget * const myTable, const QString &key, const QString &value, const UtilFRequest::FormKeyValueType type){
    Util::TableWidget::addRow(myTable, QStringList() << key << value << UtilFRequest::getFormKeyTypeString(type));

    // Set type as not editable
    int tableSize = myTable->rowCount();
    QTableWidgetItem* const addedRowTypeItem = myTable->item(tableSize-1, 2);
    addedRowTypeItem->setFlags(addedRowTypeItem->flags() & (~Qt::ItemIsEditable));

    // If it is a file, change the row color to blue in order to differentiate
    if(type == UtilFRequest::FormKeyValueType::FILE){
        myTable->item(tableSize-1, 0)->setTextColor(Qt::blue);
        myTable->item(tableSize-1, 1)->setTextColor(Qt::blue);
        addedRowTypeItem->setTextColor(Qt::blue);
    }
}

// Return original content in case of error
QString getStringFormattedForSerializationType(const QString &content, const SerializationFormatType serializationType){
    switch(serializationType){
    case UtilFRequest::SerializationFormatType::JSON:
    {
        QJsonParseError parseError;

        QJsonDocument auxJsonDoc = QJsonDocument::fromJson(content.toUtf8(), &parseError);

        if(parseError.error != QJsonParseError::NoError){
            QString errorMessage = "An error occurred while formatting the content as Json: " + content.left(10);
            Util::Dialogs::showError(errorMessage);
            LOG_ERROR << errorMessage;
            return content;
        }

        return auxJsonDoc.toJson();
    }
    case UtilFRequest::SerializationFormatType::XML:
    {
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_string(QSTR_TO_CSTR(content));

        if(result.status != pugi::xml_parse_status::status_ok){
            QString errorMessage = "An error occurred while formatting the content as XML: " + content.left(10);
            Util::Dialogs::showError(errorMessage);
            LOG_ERROR << errorMessage;
            return content;
        }

        std::stringstream xmlFormattedText;
        doc.save(xmlFormattedText);

        return QString::fromStdString(xmlFormattedText.str());
    }
    case UtilFRequest::SerializationFormatType::UNKNOWN:
    {
        return content;
    }
    default:
    {
        QString errorMessage = "Invalid currRequestFormatType " + QString::number(static_cast<int>(serializationType)) + "'. Program can't proceed.";
        Util::Dialogs::showError(errorMessage);
        LOG_FATAL << errorMessage;
        exit(1);
    }
    }
}

SerializationFormatType getSerializationFormatTypeForString(const QString &content){
    // Try to parse the response as json, if it fails try xml, otherwise return unknown
    QJsonParseError parseError;

    QJsonDocument auxJsonDoc = QJsonDocument::fromJson(content.toUtf8(), &parseError);

    if(parseError.error == QJsonParseError::NoError){
        return UtilFRequest::SerializationFormatType::JSON;
    }

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(QSTR_TO_CSTR(QString(content)));

    if(result.status == pugi::xml_parse_status::status_ok){
        return UtilFRequest::SerializationFormatType::XML;
    }

    return	UtilFRequest::SerializationFormatType::UNKNOWN;
}

QByteArray simpleStringObfuscationDeobfuscation(const QString& ofuscationSalt, const QString &input){

	QByteArray saltByteArray = ofuscationSalt.toUtf8();
    QByteArray inputByteArray = input.toUtf8();

    // Using unsigned types as they have a defined truncation in iso c++:
    // https://stackoverflow.com/a/34886065
    // (so the static_cast<unsigned char> below works the same in all different systems)
    uint32_t saltByteArraySize = static_cast<uint32_t>(saltByteArray.size());
    uint32_t inputByteArraySize = static_cast<uint32_t>(inputByteArray.size());

    for(uint32_t i=0; i< inputByteArraySize; i++){
        inputByteArray[i] = inputByteArray[i] ^ (i < saltByteArraySize ? saltByteArray[i] : static_cast<unsigned char>(i));
    }

    return inputByteArray;
}

QString replaceFRequestAuthenticationPlaceholders(const QString &textToReplace, const QString &username, const QString &password){
	return QString(textToReplace).
	replace(GlobalVars::FRequestAuthenticationPlaceholderUsername, username).
	replace(GlobalVars::FRequestAuthenticationPlaceholderPassword, password);
}

}
