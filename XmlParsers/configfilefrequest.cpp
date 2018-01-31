/*
 *
Copyright (C) 2017-2018  Fábio Bento (fabiobento512)

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

#include "configfilefrequest.h"

ConfigFileFRequest::ConfigFileFRequest(const QString &fileFullPath)
    :fileFullPath(fileFullPath)
{
    if(!QFile::exists(fileFullPath)){
        createNewConfig();
        return;
    }

    readSettingsFromFile();
}

void ConfigFileFRequest::createNewConfig(){

    pugi::xml_document doc;

    pugi::xml_node rootNode = doc.append_child("FRequestConfig");
    rootNode.append_attribute("frequestVersion").set_value(QSTR_TO_CSTR(GlobalVars::LastCompatibleVersionConfig));

    pugi::xml_node generalNode = rootNode.append_child("General");

    generalNode.append_attribute("requestTimeout").set_value(currentSettings.requestTimeout);
	generalNode.append_attribute("maxRequestResponseDataSizeToDisplay").set_value(currentSettings.maxRequestResponseDataSizeToDisplay);
    generalNode.append_attribute("onStartupOption").set_value(static_cast<int>(currentSettings.onStartupSelectedOption));
    generalNode.append_attribute("lastProjectPath");
    generalNode.append_attribute("lastResponseFilePath");
    generalNode.append_attribute("showRequestTypesIcons");

    pugi::xml_node recentProjectsNode = rootNode.append_child("RecentProjects");

    for(int i=1; i<=GlobalVars::AppRecentProjectsMaxSize; i++){
        recentProjectsNode.append_child(QSTR_TO_CSTR("RecentProject" + QString::number(i)));
    }

    pugi::xml_node proxyNode = rootNode.append_child("Proxy");

    proxyNode.append_attribute("useProxy").set_value(false);
    proxyNode.append_attribute("proxyType").set_value(0);
    proxyNode.append_attribute("hostName");
    proxyNode.append_attribute("port").set_value(0);

    pugi::xml_node windowsGeometryNode = rootNode.append_child("WindowsGeometry");

    windowsGeometryNode.append_attribute("saveWindowsGeometryWhenExiting").set_value(currentSettings.windowsGeometry.saveWindowsGeometryWhenExiting);
    pugi::xml_node mainWindowNode = windowsGeometryNode.append_child("MainWindow");
    mainWindowNode.append_child("MainWindowGeometry");
    mainWindowNode.append_child("RequestsSplitterState");
    mainWindowNode.append_child("RequestResponseSplitterState");

    pugi::xml_node defaultHeadersNode = rootNode.append_child("DefaultHeaders");
    defaultHeadersNode.append_attribute("useDefaultHeaders").set_value(currentSettings.defaultHeaders.useDefaultHeaders);

    if(!doc.save_file(QSTR_TO_CSTR(fileFullPath), PUGIXML_TEXT("\t"), pugi::format_default | pugi::format_write_bom, pugi::xml_encoding::encoding_utf8)){
        QString errorMessage = "An error ocurred while creating the FRequest config file. Application needs to abort.";
        LOG_FATAL << errorMessage;
        Util::Dialogs::showError(errorMessage);
        exit(1);
    }

    LOG_INFO << "Couldn't find an FRequest config file. A new config file was created.";

}

void ConfigFileFRequest::saveSettings(ConfigFileFRequest::Settings &newSettings){
    try
    {
        pugi::xml_document doc;

        doc.load_file(QSTR_TO_CSTR(this->fileFullPath));

        pugi::xml_node generalNode = doc.select_single_node("/FRequestConfig/General").node();

        generalNode.attribute("requestTimeout").set_value(newSettings.requestTimeout);
		generalNode.attribute("maxRequestResponseDataSizeToDisplay").set_value(newSettings.maxRequestResponseDataSizeToDisplay);
        generalNode.attribute("onStartupOption").set_value(static_cast<int>(newSettings.onStartupSelectedOption));
        generalNode.attribute("lastProjectPath").set_value(QSTR_TO_CSTR(newSettings.lastProjectPath));
        generalNode.attribute("lastResponseFilePath").set_value(QSTR_TO_CSTR(newSettings.lastResponseFilePath));
        generalNode.attribute("showRequestTypesIcons").set_value(newSettings.showRequestTypesIcons);

        pugi::xml_node recentProjectsNode = doc.select_single_node("/FRequestConfig/RecentProjects").node();

        for(int i=1; i<=GlobalVars::AppRecentProjectsMaxSize && newSettings.recentProjectsPaths.size() >= i; i++){
            recentProjectsNode.child(QSTR_TO_CSTR("RecentProject" + QString::number(i))).text().set(QSTR_TO_CSTR(newSettings.recentProjectsPaths.at(i-1)));
        }

        pugi::xml_node proxyNode = doc.select_single_node("/FRequestConfig/Proxy").node();

        proxyNode.attribute("useProxy").set_value(newSettings.useProxy);
        proxyNode.attribute("proxyType").set_value(static_cast<int>(newSettings.proxySettings.type));
        proxyNode.attribute("hostName").set_value(QSTR_TO_CSTR(newSettings.proxySettings.hostName));
        proxyNode.attribute("port").set_value(newSettings.proxySettings.portNumber);

        pugi::xml_node windowsGeometryNode = doc.select_single_node("/FRequestConfig/WindowsGeometry").node();

        windowsGeometryNode.attribute("saveWindowsGeometryWhenExiting").set_value(newSettings.windowsGeometry.saveWindowsGeometryWhenExiting);

        pugi::xml_node mainWindowNode = windowsGeometryNode.child("MainWindow");

        mainWindowNode.child("MainWindowGeometry").text().set(QSTR_TO_CSTR(QString(newSettings.windowsGeometry.mainWindow_MainWindowGeometry.toBase64())));
        mainWindowNode.child("RequestsSplitterState").text().set(QSTR_TO_CSTR(QString(newSettings.windowsGeometry.mainWindow_RequestsSplitterState.toBase64())));
        mainWindowNode.child("RequestResponseSplitterState").text().set(QSTR_TO_CSTR(QString(newSettings.windowsGeometry.mainWindow_RequestResponseSplitterState.toBase64())));

        pugi::xml_node defaultHeadersNode = doc.select_single_node("/FRequestConfig/DefaultHeaders").node();

        defaultHeadersNode.attribute("useDefaultHeaders").set_value(newSettings.defaultHeaders.useDefaultHeaders);

        auto fUpdateHeaders = [](pugi::xml_node &currentHeaderNode, const QVector<UtilFRequest::HttpHeader> &headers, const char *nodeName){

            // Remove current Headers to update with new ones
            currentHeaderNode.remove_child(nodeName);
            pugi::xml_node rawNode = currentHeaderNode.append_child(nodeName);

            for(const UtilFRequest::HttpHeader &currentHeader : headers){
                pugi::xml_node headerNode = rawNode.append_child("Header");
                headerNode.append_child("Key").append_child(pugi::xml_node_type::node_cdata).text().set(QSTR_TO_CSTR(currentHeader.name));
                headerNode.append_child("Value").append_child(pugi::xml_node_type::node_cdata).text().set(QSTR_TO_CSTR(currentHeader.value));
            }
        };

        auto fWriteRequestType = [&fUpdateHeaders, &defaultHeadersNode](const QString &requestText, const std::experimental::optional<ConfigFileFRequest::ProtocolHeader> &requestHeaders, const bool hasBody){
            if(requestHeaders.has_value()){

                pugi::xml_node currentHeaderNode = defaultHeadersNode.child(QSTR_TO_CSTR(requestText));

                if(currentHeaderNode.empty()){
                    currentHeaderNode = defaultHeadersNode.append_child(QSTR_TO_CSTR(requestText));
                }

                if(requestHeaders.value().headers_Raw.has_value()){
                    fUpdateHeaders(currentHeaderNode, requestHeaders.value().headers_Raw.value(), "Raw");
                }

                if(hasBody){
                    if(requestHeaders.value().headers_Form_Data.has_value()){
                        fUpdateHeaders(currentHeaderNode, requestHeaders.value().headers_Form_Data.value(), "Form-data");
                    }

                    if(requestHeaders.value().headers_X_form_www_urlencoded.has_value()){
                        fUpdateHeaders(currentHeaderNode, requestHeaders.value().headers_X_form_www_urlencoded.value(), "X-form-www-urlencoded");
                    }
                }

            }
        };

        for(const UtilFRequest::RequestType &currentRequestType : UtilFRequest::possibleRequestTypes){
            QString currentRequestTypeText = UtilFRequest::getRequestTypeString(currentRequestType);
            bool hasBody = UtilFRequest::requestTypeMayHaveBody(currentRequestType);
            const std::experimental::optional<ConfigFileFRequest::ProtocolHeader> &requestHeaders = getSettingsHeaderForRequestType(currentRequestType, newSettings);

            fWriteRequestType(currentRequestTypeText, requestHeaders, hasBody);
        }

        // Remove current config authentications (if they exist), since we will write all of them again
        doc.select_single_node("/FRequestConfig").node().remove_child("ConfigProjectAuthentications");

        pugi::xml_node configAuthNode = doc.select_single_node("/FRequestConfig").node().append_child("ConfigProjectAuthentications");

        for(const ConfigurationProjectAuthentication &currentConfigAuth : newSettings.mapOfConfigAuths_UuidToConfigAuth){

            const FRequestAuthentication &authData = *currentConfigAuth.authData;

            pugi::xml_node currentAuth = configAuthNode.append_child("Authentication");
            currentAuth.append_attribute("lastProjectName").set_value(QSTR_TO_CSTR(currentConfigAuth.lastProjectName));
            currentAuth.append_attribute("projectUuid").set_value(QSTR_TO_CSTR(currentConfigAuth.projectUuid));
            currentAuth.append_attribute("type").set_value(static_cast<int>(authData.type));
            currentAuth.append_attribute("bRetryLoginIfError").set_value(authData.retryLoginIfError401);

            switch(authData.type){
            case FRequestAuthentication::AuthenticationType::REQUEST_AUTHENTICATION:
            {
                const RequestAuthentication &requestAuth = static_cast<const RequestAuthentication&>(authData);

                currentAuth.append_attribute("requestUuid").set_value(QSTR_TO_CSTR(requestAuth.requestForAuthenticationUuid));
                currentAuth.append_child("Username").append_child(pugi::xml_node_type::node_pcdata).set_value(QSTR_TO_CSTR(requestAuth.username));
                currentAuth.append_child("PasswordSalt").append_child(pugi::xml_node_type::node_pcdata).set_value(QSTR_TO_CSTR(requestAuth.passwordSalt));
                currentAuth.append_child("Password").append_child(pugi::xml_node_type::node_pcdata).
                        set_value((QSTR_TO_CSTR(QString(UtilFRequest::simpleStringObfuscationDeobfuscation(requestAuth.passwordSalt, requestAuth.password).toBase64()))));
                break;
            }
            case FRequestAuthentication::AuthenticationType::BASIC_AUTHENTICATION:
            {
                const BasicAuthentication &basicAuth = static_cast<const BasicAuthentication&>(authData);
                currentAuth.append_child("Username").append_child(pugi::xml_node_type::node_pcdata).set_value(QSTR_TO_CSTR(basicAuth.username));
                currentAuth.append_child("PasswordSalt").append_child(pugi::xml_node_type::node_pcdata).set_value(QSTR_TO_CSTR(basicAuth.passwordSalt));
                currentAuth.append_child("Password").append_child(pugi::xml_node_type::node_pcdata).
                        set_value(QSTR_TO_CSTR(QString(UtilFRequest::simpleStringObfuscationDeobfuscation(basicAuth.passwordSalt, basicAuth.password).toBase64())));

                break;
            }
            default:
            {
                QString errorMessage = "Authentication type unknown: '" + QString::number(static_cast<int>(authData.type)) + "'. Program can't proceed.";
                Util::Dialogs::showError(errorMessage);
                LOG_FATAL << errorMessage;
                exit(1);
            }
            }
        }
        if(!doc.save_file(QSTR_TO_CSTR(fileFullPath), PUGIXML_TEXT("\t"), pugi::format_default | pugi::format_write_bom, pugi::xml_encoding::encoding_utf8)){
            throw std::runtime_error(QSTR_TO_CSTR("Error while saving: '" + fileFullPath + "'"));
        }
    }
    catch(const std::exception &e)
    {
        QString errorMessage = "An error ocurred while saving the FRequest settings. Settings were not saved." + QString(e.what());
        LOG_ERROR << errorMessage;
        Util::Dialogs::showError(errorMessage);
    }

    this->currentSettings = newSettings;
}

void ConfigFileFRequest::readSettingsFromFile(){
    try
    {
        upgradeConfigFileIfNecessary();

        pugi::xml_document doc;

        doc.load_file(QSTR_TO_CSTR(this->fileFullPath));

        pugi::xml_node generalNode = doc.select_single_node("/FRequestConfig/General").node();

        this->currentSettings.requestTimeout = generalNode.attribute("requestTimeout").as_uint();
		this->currentSettings.maxRequestResponseDataSizeToDisplay = generalNode.attribute("maxRequestResponseDataSizeToDisplay").as_uint();
        this->currentSettings.onStartupSelectedOption = static_cast<OnStartupOption>(generalNode.attribute("onStartupOption").as_int());
        this->currentSettings.lastProjectPath = generalNode.attribute("lastProjectPath").as_string();
        this->currentSettings.lastResponseFilePath = generalNode.attribute("lastResponseFilePath").as_string();
        this->currentSettings.showRequestTypesIcons = generalNode.attribute("showRequestTypesIcons").as_bool();

        pugi::xml_node recentProjectsNode = doc.select_single_node("/FRequestConfig/RecentProjects").node();

        this->currentSettings.recentProjectsPaths.clear();

        for(int i=1; i<=GlobalVars::AppRecentProjectsMaxSize; i++){
            if(!recentProjectsNode.child(QSTR_TO_CSTR("RecentProject" + QString::number(i))).empty()){
                if(!QString(recentProjectsNode.child(QSTR_TO_CSTR("RecentProject" + QString::number(i))).text().as_string()).isEmpty()){
                    this->currentSettings.recentProjectsPaths.append(recentProjectsNode.child(QSTR_TO_CSTR("RecentProject" + QString::number(i))).text().as_string());
                }
            }
        }

        pugi::xml_node proxyNode = doc.select_single_node("/FRequestConfig/Proxy").node();

        this->currentSettings.useProxy = proxyNode.attribute("useProxy").as_bool(false);
        this->currentSettings.proxySettings.type = static_cast<ProxyType>(proxyNode.attribute("proxyType").as_int(0));
        this->currentSettings.proxySettings.hostName = proxyNode.attribute("hostName").as_string();
        this->currentSettings.proxySettings.portNumber = proxyNode.attribute("port").as_uint(0);

        pugi::xml_node windowsGeometryNode = doc.select_single_node("/FRequestConfig/WindowsGeometry").node();

        this->currentSettings.windowsGeometry.saveWindowsGeometryWhenExiting = windowsGeometryNode.attribute("saveWindowsGeometryWhenExiting").as_bool();

        pugi::xml_node mainWindowNode = windowsGeometryNode.child("MainWindow");

        auto fReadBase64Setting = [&mainWindowNode](const char * const node) -> QByteArray{
            QByteArray auxBase64Decode;
            auxBase64Decode.append(QString(mainWindowNode.child(node).text().as_string()));

            return QByteArray::fromBase64(auxBase64Decode);
        };

        this->currentSettings.windowsGeometry.mainWindow_MainWindowGeometry = fReadBase64Setting("MainWindowGeometry");
        this->currentSettings.windowsGeometry.mainWindow_RequestsSplitterState = fReadBase64Setting("RequestsSplitterState");
        this->currentSettings.windowsGeometry.mainWindow_RequestResponseSplitterState = fReadBase64Setting("RequestResponseSplitterState");

        pugi::xml_node defaultHeadersNode = doc.select_single_node("/FRequestConfig/DefaultHeaders").node();

        this->currentSettings.defaultHeaders.useDefaultHeaders = defaultHeadersNode.attribute("useDefaultHeaders").as_bool();

        auto fReadHeaders = [](UtilFRequest::BodyType bodyType, std::experimental::optional<ConfigFileFRequest::ProtocolHeader> &requestHeaders, pugi::xml_node &currentHeaderNode){

            QString nodeName;

            switch(bodyType){
            case UtilFRequest::BodyType::RAW:
                nodeName = "Raw";
                break;
            case UtilFRequest::BodyType::FORM_DATA:
                nodeName = "Form-data";
                break;
            case UtilFRequest::BodyType::X_FORM_WWW_URLENCODED:
                nodeName = "X-form-www-urlencoded";
                break;
            default:
                throw std::runtime_error(QSTR_TO_CSTR("Unknown body type " + QString::number(static_cast<int>(bodyType))));
            }

            for(const pugi::xml_node &currentNode : currentHeaderNode.child(QSTR_TO_CSTR(nodeName)).children()){

                if(!requestHeaders.has_value()){
                    requestHeaders = ProtocolHeader();
                }

                std::experimental::optional<QVector<UtilFRequest::HttpHeader>> *currentHeaders;

                switch(bodyType){
                case UtilFRequest::BodyType::RAW:
                    currentHeaders = &requestHeaders.value().headers_Raw;
                    break;
                case UtilFRequest::BodyType::FORM_DATA:
                    currentHeaders = &requestHeaders.value().headers_Form_Data;
                    break;
                case UtilFRequest::BodyType::X_FORM_WWW_URLENCODED:
                    currentHeaders = &requestHeaders.value().headers_X_form_www_urlencoded;
                    break;
                default:
                    throw std::runtime_error(QSTR_TO_CSTR("Unknown body type " + QString::number(static_cast<int>(bodyType))));
                }

                if(!currentHeaders->has_value()){
                    (*currentHeaders) = QVector<UtilFRequest::HttpHeader>();
                }

                UtilFRequest::HttpHeader currentHeader;
                currentHeader.name = currentNode.child("Key").text().as_string();
                currentHeader.value = currentNode.child("Value").text().as_string();

                (*currentHeaders).value().append(currentHeader);
            }
        };

        auto fReadRequestType = [&defaultHeadersNode, &fReadHeaders](const QString &requestText, std::experimental::optional<ConfigFileFRequest::ProtocolHeader> &requestHeaders, const bool hasBody){

            pugi::xml_node currentHeaderNode = defaultHeadersNode.child(QSTR_TO_CSTR(requestText));

            if(!currentHeaderNode.empty()){

                if(!requestHeaders.has_value()){
                    requestHeaders = ProtocolHeader();
                }

                if(!currentHeaderNode.child("Raw").empty()){
                    fReadHeaders(UtilFRequest::BodyType::RAW, requestHeaders, currentHeaderNode);
                }

                if(hasBody){
                    if(!currentHeaderNode.child("Form-data").empty()){
                        fReadHeaders(UtilFRequest::BodyType::FORM_DATA, requestHeaders, currentHeaderNode);
                    }

                    if(!currentHeaderNode.child("X-form-www-urlencoded").empty()){
                        fReadHeaders(UtilFRequest::BodyType::X_FORM_WWW_URLENCODED, requestHeaders, currentHeaderNode);
                    }
                }
            }
        };

        for(const UtilFRequest::RequestType &currentRequestType : UtilFRequest::possibleRequestTypes){
            QString currentRequestTypeText = UtilFRequest::getRequestTypeString(currentRequestType);
            bool hasBody = UtilFRequest::requestTypeMayHaveBody(currentRequestType);
            std::experimental::optional<ConfigFileFRequest::ProtocolHeader> &requestHeaders = getSettingsHeaderForRequestType(currentRequestType, this->currentSettings);

            fReadRequestType(currentRequestTypeText, requestHeaders, hasBody);
        }

        // Check if there is authentication data and load it
        pugi::xpath_node_set configAuthNodes = doc.select_nodes("/FRequestConfig/ConfigProjectAuthentications/Authentication");

        for(const pugi::xpath_node &currXPathNode : configAuthNodes){

            pugi::xml_node currAuthNode = currXPathNode.node();
            ConfigurationProjectAuthentication currProjAuth;
            currProjAuth.lastProjectName = currAuthNode.attribute("lastProjectName").value();
            currProjAuth.projectUuid = currAuthNode.attribute("projectUuid").value();

            FRequestAuthentication::AuthenticationType authType =
                    static_cast<FRequestAuthentication::AuthenticationType>(currAuthNode.attribute("type").as_int());

            const bool retryLoginIfError401 = currAuthNode.attribute("bRetryLoginIfError").as_bool();

            switch(authType){
            case FRequestAuthentication::AuthenticationType::REQUEST_AUTHENTICATION:
            {
                QString username = currAuthNode.child("Username").child_value();
                QString passwordSalt = currAuthNode.child("PasswordSalt").child_value();
                QString password = QString(UtilFRequest::simpleStringObfuscationDeobfuscation(passwordSalt,
                                                                                              QByteArray::fromBase64(currAuthNode.child("Password").child_value())));
                QString requestUuid = currAuthNode.attribute("requestUuid").value();

                currProjAuth.authData = std::make_shared<RequestAuthentication>(RequestAuthentication(true, retryLoginIfError401, username, passwordSalt, password, requestUuid));
                break;
            }
            case FRequestAuthentication::AuthenticationType::BASIC_AUTHENTICATION:
            {
                QString username = currAuthNode.child("Username").child_value();
                QString passwordSalt = currAuthNode.child("PasswordSalt").child_value();
                QString password = QString(UtilFRequest::simpleStringObfuscationDeobfuscation(passwordSalt,
                                                                                              QByteArray::fromBase64(currAuthNode.child("Password").child_value())));

                currProjAuth.authData = std::make_shared<BasicAuthentication>(BasicAuthentication(true, retryLoginIfError401, username, passwordSalt, password));
                break;
            }
            default:
            {
                QString errorMessage = "Authentication type unknown: '" + QString::number(static_cast<int>(authType)) + "'. Program can't proceed.";
                Util::Dialogs::showError(errorMessage);
                LOG_FATAL << errorMessage;
                exit(1);
            }
            }

            this->currentSettings.mapOfConfigAuths_UuidToConfigAuth.insert(currProjAuth.projectUuid, currProjAuth);
        }

        if(!doc.save_file(QSTR_TO_CSTR(fileFullPath), PUGIXML_TEXT("\t"), pugi::format_default | pugi::format_write_bom, pugi::xml_encoding::encoding_utf8)){
            throw std::runtime_error(QSTR_TO_CSTR("Error while saving: '" + fileFullPath + "'"));
        }
    }
    catch(const std::exception &e)
    {
        QString errorMessage = "An error ocurred while loading the FRequest settings. Settings were not loaded. " + QString(e.what());
        LOG_ERROR << errorMessage;
        Util::Dialogs::showError(errorMessage);
    }
}

ConfigFileFRequest::Settings ConfigFileFRequest::getCurrentSettings(){
    return this->currentSettings;
}

std::experimental::optional<ConfigFileFRequest::ProtocolHeader>& ConfigFileFRequest::getSettingsHeaderForRequestType(UtilFRequest::RequestType currentRequestType, Settings &currentSettings){

    std::experimental::optional<ProtocolHeader> *currentProtocolHeader = nullptr;

    switch(currentRequestType){
    case UtilFRequest::RequestType::GET_OPTION:
    {
        currentProtocolHeader = &currentSettings.defaultHeaders.getHeaders;
        break;
    }
    case UtilFRequest::RequestType::POST_OPTION:
    {
        currentProtocolHeader = &currentSettings.defaultHeaders.postHeaders;
        break;
    }
    case UtilFRequest::RequestType::PUT_OPTION:
    {
        currentProtocolHeader = &currentSettings.defaultHeaders.putHeaders;
        break;
    }
    case UtilFRequest::RequestType::DELETE_OPTION:
    {
        currentProtocolHeader = &currentSettings.defaultHeaders.deleteHeaders;
        break;
    }
    case UtilFRequest::RequestType::PATCH_OPTION:
    {
        currentProtocolHeader = &currentSettings.defaultHeaders.patchHeaders;
        break;
    }
    case UtilFRequest::RequestType::HEAD_OPTION:
    {
        currentProtocolHeader = &currentSettings.defaultHeaders.headHeaders;
        break;
    }
    case UtilFRequest::RequestType::TRACE_OPTION:
    {
        currentProtocolHeader = &currentSettings.defaultHeaders.traceHeaders;
        break;
    }
    case UtilFRequest::RequestType::OPTIONS_OPTION:
    {
        currentProtocolHeader = &currentSettings.defaultHeaders.optionsHeaders;
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

    return *currentProtocolHeader;
}

void ConfigFileFRequest::upgradeConfigFileIfNecessary(){

    pugi::xml_document doc;

    pugi::xml_parse_result result = doc.load_file(QSTR_TO_CSTR(this->fileFullPath));

    if(result.status!=pugi::status_ok){
        throw std::runtime_error(QSTR_TO_CSTR(QString("An error ocurred while loading project file.\n") + result.description()));
    }

    QString configVersion = QString(doc.select_single_node("/FRequestConfig").node().attribute("frequestVersion").as_string());

    if(configVersion != GlobalVars::LastCompatibleVersionConfig){
        if(!Util::FileSystem::backupFile(this->fileFullPath, this->fileFullPath + UtilFRequest::getDateTimeFormatForFilename(QDateTime::currentDateTime()))){
            QString errorMessage = "Couldn't backup the existing config file for version upgrade, program can't proceed.";
            Util::Dialogs::showError(errorMessage);
            LOG_FATAL << errorMessage;
            exit(1);
        }
    }
	
    auto fSaveFileAfterUpgrade = [&doc](const QString &fileFullPath, const QString &upgradeVersion){
        if(!doc.save_file(QSTR_TO_CSTR(fileFullPath), PUGIXML_TEXT("\t"), pugi::format_default | pugi::format_write_bom, pugi::xml_encoding::encoding_utf8)){
            throw std::runtime_error(QSTR_TO_CSTR("Error while saving: '" + fileFullPath + "'. After file version upgrade. (to version " + upgradeVersion + " )"));
        }
	};
	
    if(configVersion == "1.0"){

		const QString versionAfterUpgrade = "1.1";

        doc.select_single_node("/FRequestConfig").node().attribute("frequestVersion").set_value(QSTR_TO_CSTR(versionAfterUpgrade));

        pugi::xml_node generalNode = doc.select_single_node("/FRequestConfig/General").node();

        generalNode.append_attribute("showRequestTypesIcons").set_value(true);

        // Fix proxy node (it should start with upper case instead of lower case

        pugi::xml_node proxyNode = doc.select_single_node("/FRequestConfig/proxy").node();

        if(!proxyNode.empty()){
            proxyNode.set_name("Proxy");
        }

        fSaveFileAfterUpgrade(this->fileFullPath, versionAfterUpgrade);
		
		configVersion = versionAfterUpgrade;
    }
	if(configVersion == "1.1"){
		const QString versionAfterUpgrade = "1.1a";

        doc.select_single_node("/FRequestConfig").node().attribute("frequestVersion").set_value(QSTR_TO_CSTR(versionAfterUpgrade));

        pugi::xml_node generalNode = doc.select_single_node("/FRequestConfig/General").node();

		// Delete old attribute 'askToOpenLastProject'
        pugi::xml_attribute askToOpenLastProjectAttribute = generalNode.attribute("askToOpenLastProject");

        if(!askToOpenLastProjectAttribute.empty()){
            generalNode.remove_attribute(askToOpenLastProjectAttribute);
        }
		
		// Add new attribute that replaces 'askToOpenLastProject'
		
		// use ASK_TO_LOAD_LAST_PROJECT as default
		generalNode.append_attribute("onStartupOption").set_value(static_cast<int>(OnStartupOption::ASK_TO_LOAD_LAST_PROJECT)); 
		
		// Add new attribute that specifies the max response size for display in ui
		generalNode.append_attribute("maxRequestResponseDataSizeToDisplay").set_value(200); // 200 kb default
		
        fSaveFileAfterUpgrade(this->fileFullPath, versionAfterUpgrade);
		
		configVersion = versionAfterUpgrade;
	}
	
    if(configVersion != GlobalVars::LastCompatibleVersionConfig){
        throw std::runtime_error("Can't load the config file, it is from an incompatible version. Probably newer?");
    }
}
