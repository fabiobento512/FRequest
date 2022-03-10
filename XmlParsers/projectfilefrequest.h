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

#ifndef PROJECTFILEFREQUEST_H
#define PROJECTFILEFREQUEST_H

#include <QUuid>
#include <memory>

#include "utilfrequest.h"
#include "Authentications/requestauthentication.h"
#include "Authentications/basicauthentication.h"

class ProjectFileFRequest
{
public:

    struct ProjectData{
        QString projectName;
        QString mainUrl;
        QVector<UtilFRequest::RequestInfo> projectRequests;
        QString projectUuid;
        std::shared_ptr<FRequestAuthentication> authData = nullptr;
        bool retryLoginIfError401 = false;
        UtilFRequest::IdentCharacter saveIdentCharacter;
        QVector<UtilFRequest::HttpHeader> globalHeaders;
    };

public:
    ProjectFileFRequest() = delete;
    static ProjectFileFRequest::ProjectData readProjectDataFromFile(const QString &fileFullPath);
    static void saveProjectDataToFile(const QString &fileFullPath, const ProjectData &newProjectData, const QVector<QString> &uuidsToCleanUp);
private:
    static pugi::xml_attribute createOrGetPugiXmlAttribute(pugi::xml_node &mainNode, const char *name);
    static void upgradeProjectFileIfNecessary(const QString &filePath);
};

namespace pugiIdentChars {
    static constexpr pugi::char_t spaceChar[] = PUGIXML_TEXT("    "); // we use 4 spaces as default
    static constexpr pugi::char_t tabChar[] = PUGIXML_TEXT("\t");

    const pugi::char_t* getIdentCharaterForEnum(const UtilFRequest::IdentCharacter identEnum);
}

#endif // PROJECTFILEFREQUEST_H
