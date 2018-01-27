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

#ifndef FREQUESTAUTHENTICATION_H
#define FREQUESTAUTHENTICATION_H

#include "util.h"

// PLog library (log library)
// https://github.com/SergiusTheBest/plog
#include <plog/Log.h>

class FRequestAuthentication
{

public:
	enum class AuthenticationType{
		REQUEST_AUTHENTICATION = 0,
		BASIC_AUTHENTICATION = 1
	};

protected:
    FRequestAuthentication(const bool saveAuthToConfigFile, const bool retryLoginIfError401, const AuthenticationType type);

public:
    virtual ~FRequestAuthentication() = default; // needed to avoid undefined behaviour https://stackoverflow.com/a/22491471/1499019
	static AuthenticationType getAuthenticationTypeByString(const QString &currentAuthenticationTypeText);
	static QString getAuthenticationString(const AuthenticationType currentAuthType);
	
public:
	const AuthenticationType type;
	const bool saveAuthToConfigFile; // otherwise saves for project file
	const bool retryLoginIfError401; // should we retry on error 401?
};

#endif // FREQUESTAUTHENTICATION_H
