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

#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QTimer>

#include <util.h>

#include "utilglobalvars.h"
#include "proxysetup.h"

class UpdateChecker: public QObject /* inheritance needed for signal / slots */
{
	Q_OBJECT /* Q_OBJECT needed for signal / slots */
public:
	static void startNewInstance(const ConfigFileFRequest::Settings &settings);
private:
	// not meant to be instantiated directly, use startNewInstance instead
	UpdateChecker(const ConfigFileFRequest::Settings &settings);
private:
	void checkForUpdates();
	void replyFinished(QNetworkReply *reply);
	void checkForQNetworkAccessManagerTimeout(QNetworkReply *reply);
private:
	QNetworkAccessManager networkAccessManager;
	QNetworkRequest networkRequest;
	const ConfigFileFRequest::Settings settings;
	const QString updateCheckApiUrl = "https://api.github.com/repos/fabiobento512/FRequest/releases/latest";
	const QString releasesUrl = "https://github.com/fabiobento512/FRequest/releases";
    bool replyHasFinished = false;
};

#endif // UPDATECHECKER_H
