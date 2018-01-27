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

#include "requestauthentication.h"

RequestAuthentication::RequestAuthentication
(
const bool saveAuthToConfigFile, 
const bool retryLoginIfError401, 
const QString &username, 
const QString &passwordSalt, 
const QString &password,
const QString requestForAuthenticationUuid
)
: FRequestAuthentication(saveAuthToConfigFile, retryLoginIfError401, AuthenticationType::REQUEST_AUTHENTICATION), 
	username(username), 
	passwordSalt(passwordSalt), 
	password(password),
	requestForAuthenticationUuid(requestForAuthenticationUuid)
{
	
}
