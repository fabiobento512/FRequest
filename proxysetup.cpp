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

#include "proxysetup.h"

void ProxySetup::setupProxyForNetworkManager(const ConfigFileFRequest::Settings &settings, QNetworkAccessManager * const networkAccessManager){
	QNetworkProxy proxy;

	if(settings.useProxy){
		switch (settings.proxySettings.type) {
		case ConfigFileFRequest::ProxyType::AUTOMATIC:
		{
			QNetworkProxyFactory::setUseSystemConfiguration(true);
			break;
		}
		case ConfigFileFRequest::ProxyType::HTTP_TRANSPARENT:
		{
			proxy.setType(QNetworkProxy::HttpProxy);
			break;
		}
		case ConfigFileFRequest::ProxyType::HTTP:
		{
			proxy.setType(QNetworkProxy::HttpCachingProxy);
			break;
		}
		case ConfigFileFRequest::ProxyType::SOCKS5:
		{
			proxy.setType(QNetworkProxy::Socks5Proxy);
			break;
		}
		default:
		{
			QString errorMessage = "Unknown proxy type set! '" + QString::number(static_cast<unsigned int>(settings.proxySettings.type)) + "'. Check the proxy settings.";
			Util::Dialogs::showError(errorMessage);
			LOG_ERROR << errorMessage;
			return;
		}
		}

		if(settings.proxySettings.type != ConfigFileFRequest::ProxyType::AUTOMATIC){
			QNetworkProxyFactory::setUseSystemConfiguration(false);
			proxy.setHostName(settings.proxySettings.hostName);
			proxy.setPort(settings.proxySettings.portNumber);
		}
	}
	else{
		proxy.setType(QNetworkProxy::NoProxy);
	}
	
	networkAccessManager->setProxy(proxy);
}