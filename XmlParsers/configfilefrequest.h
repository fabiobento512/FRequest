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

#ifndef CONFIGFILEFREQUEST_H
#define CONFIGFILEFREQUEST_H

#include "utilfrequest.h"
#include "Authentications/requestauthentication.h"
#include "Authentications/basicauthentication.h"

class ConfigFileFRequest
{
public:

    enum class ProxyType{
        AUTOMATIC = 0,
        HTTP_TRANSPARENT,
        HTTP,
        SOCKS5
    };

    struct ProtocolHeader{
        std::experimental::optional<QVector<UtilFRequest::HttpHeader>> headers_Raw;
        std::experimental::optional<QVector<UtilFRequest::HttpHeader>> headers_Form_Data;
        std::experimental::optional<QVector<UtilFRequest::HttpHeader>> headers_X_form_www_urlencoded;
    };

    struct DefaultHeaders{
        bool useDefaultHeaders = false;
        std::experimental::optional<ProtocolHeader> getHeaders;
        std::experimental::optional<ProtocolHeader> postHeaders;
        std::experimental::optional<ProtocolHeader> putHeaders;
        std::experimental::optional<ProtocolHeader> deleteHeaders;
        std::experimental::optional<ProtocolHeader> patchHeaders;
        std::experimental::optional<ProtocolHeader> headHeaders;
        std::experimental::optional<ProtocolHeader> traceHeaders;
        std::experimental::optional<ProtocolHeader> optionsHeaders;
    };

    struct WindowsGeometry{
        bool saveWindowsGeometryWhenExiting = false;
        QByteArray mainWindow_MainWindowGeometry;
        QByteArray mainWindow_RequestsSplitterState;
        QByteArray mainWindow_RequestResponseSplitterState;
    };

    struct ProxySettings{
        ProxyType type = ProxyType::AUTOMATIC;
        QString hostName;
        unsigned int portNumber = 0;
    };

	struct ConfigurationProjectAuthentication{
		QString lastProjectName;
		QString projectUuid;
        std::shared_ptr<FRequestAuthentication> authData; // using shared ptr just to not be obligated to initialize all members at same time
	};
	
    struct Settings{
        unsigned int requestTimeout = 20;
        bool askToOpenLastProject = false;
        QString lastProjectPath;
        QString lastResponseFilePath;
        bool showRequestTypesIcons = true;
        QVector<QString> recentProjectsPaths;
        WindowsGeometry windowsGeometry;
        DefaultHeaders defaultHeaders;
        bool useProxy = false;
        ProxySettings proxySettings;
		// the hash map below contains the authentication data of the projects that specified it to be saved on config file instead of in project file
		QHash<QString, ConfigurationProjectAuthentication> mapOfConfigAuths_UuidToConfigAuth;
    };

public:
    ConfigFileFRequest(const QString &fileFullPath);
    ConfigFileFRequest::Settings getCurrentSettings();
    void saveSettings(Settings &newSettings);
    static std::experimental::optional<ProtocolHeader>& getSettingsHeaderForRequestType(UtilFRequest::RequestType currentRequestType, Settings &currentSettings);
private:
    void createNewConfig();
    void readSettingsFromFile();
    void upgradeConfigFileIfNecessary();
private:
    Settings currentSettings;
    const QString fileFullPath;
};

#endif // CONFIGFILEFREQUEST_H
