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

#ifndef UTILGLOBALVARS_H
#define UTILGLOBALVARS_H

namespace GlobalVars{

static const QString AppName = "FRequest";
static const QString AppVersion = "1.2";
static const QString LastCompatibleVersionConfig = "1.1b";
static const QString LastCompatibleVersionProjects= "1.1c";
static const QString AppConfigFileName = AppName + ".cfg";
static const QString AppLogFileName = AppName.toLower() + ".log";
static const QString FRequestAuthenticationPlaceholderUsername = "{{FREQUEST_AUTH_USERNAME}}";
static const QString FRequestAuthenticationPlaceholderPassword = "{{FREQUEST_AUTH_PASSWORD}}";
static constexpr int AppRecentProjectsMaxSize=6;

}

#endif // UTILGLOBALVARS_H
