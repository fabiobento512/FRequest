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

#include "frequestauthentication.h"


FRequestAuthentication::FRequestAuthentication(const bool saveAuthToConfigFile, const bool retryLoginIfError401, const AuthenticationType type)
    :type(type), saveAuthToConfigFile(saveAuthToConfigFile), retryLoginIfError401(retryLoginIfError401)
{

}

FRequestAuthentication::AuthenticationType FRequestAuthentication::getAuthenticationTypeByString(const QString &currentAuthenticationTypeText){

    if(currentAuthenticationTypeText == "Request Authentication"){
        return AuthenticationType::REQUEST_AUTHENTICATION;
    }
    else if(currentAuthenticationTypeText == "Basic Authentication"){
        return AuthenticationType::BASIC_AUTHENTICATION;
    }
    else{
        QString errorMessage = "Authentication type unknown: '" + currentAuthenticationTypeText + "'. Program can't proceed.";
        Util::Dialogs::showError(errorMessage);
        LOG_FATAL << errorMessage;
        exit(1);
    }
}

QString FRequestAuthentication::getAuthenticationString(const AuthenticationType currentAuthType){
	switch(currentAuthType){
    case AuthenticationType::REQUEST_AUTHENTICATION:
        return "Request Authentication";
    case AuthenticationType::BASIC_AUTHENTICATION:
        return "Basic Authentication";
    default:
    {
        QString errorMessage = "Invalid authentication type " + QString::number(static_cast<int>(currentAuthType)) + "'. Program can't proceed.";
        Util::Dialogs::showError(errorMessage);
        LOG_FATAL << errorMessage;
        exit(1);
    }
    }
}