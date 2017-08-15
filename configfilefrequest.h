#ifndef CONFIGFILEFREQUEST_H
#define CONFIGFILEFREQUEST_H

#include <pugixml/pugixml.hpp>
#include <cpp17optional/optional.hpp>
#include "utilfrequest.h"
#include "utilglobalvars.h"

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

    struct Settings{
        unsigned int requestTimeout = 20;
        bool askToOpenLastProject = false;
        QString lastProjectPath;
        QString lastResponseFilePath;
        QVector<QString> recentProjectsPaths;
        WindowsGeometry windowsGeometry;
        DefaultHeaders defaultHeaders;
        bool useProxy = false;
        ProxySettings proxySettings;
    };

public:
    ConfigFileFRequest(const QString &fileFullPath);
    ConfigFileFRequest::Settings getCurrentSettings();
    void saveSettings(Settings &newSettings);
    static std::experimental::optional<ProtocolHeader>& getSettingsHeaderForRequestType(UtilFRequest::RequestType currentRequestType, Settings &currentSettings);
private:
    void createNewConfig();
    void readSettingsFromFile();
private:
    Settings currentSettings;
    const QString fileFullPath;
};

#endif // CONFIGFILEFREQUEST_H
