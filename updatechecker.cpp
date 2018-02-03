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

#include "updatechecker.h"

UpdateChecker::UpdateChecker(const ConfigFileFRequest::Settings &settings)
    :settings(settings)
{
    this->networkRequest.setUrl(QUrl(this->updateCheckApiUrl));
}

void UpdateChecker::startNewInstance(const ConfigFileFRequest::Settings &settings){
    UpdateChecker *newInstance = new UpdateChecker(settings);
    newInstance->checkForUpdates(); // this deletes itself once finished
}

void UpdateChecker::checkForUpdates(){
    // Apply proxy type
    ProxySetup::setupProxyForNetworkManager(this->settings, &this->networkAccessManager);

    connect(&this->networkAccessManager, &QNetworkAccessManager::finished, this, &UpdateChecker::replyFinished);

    // do the request and also check for timeout
    checkForQNetworkAccessManagerTimeout(this->networkAccessManager.get(this->networkRequest));

    this->deleteLater(); // delete when signal / slots are finished
}

void UpdateChecker::replyFinished(QNetworkReply *reply){

    try{
        if(!this->replyHasFinished){
            if (reply->error()) {
                throw std::runtime_error(QSTR_TO_CSTR(reply->errorString()));
            }

            QString answer = reply->readAll();

            QJsonDocument doc = QJsonDocument::fromJson(QSTR_TO_CSTR(answer));

            if(!doc.isObject()){
                throw std::runtime_error("(http api error) json is not an object.");
            }

            QJsonObject jsonObj = doc.object();

            QJsonValue jsonProgramLastVersion = jsonObj.value("tag_name");

            if(jsonProgramLastVersion.isUndefined()){
                throw std::runtime_error("(http api error) json tag_name doesn't exist.");
            }

            QString newVersion = jsonProgramLastVersion.toString();

            // remove v (from v1.0 for example) if it exists
            if(newVersion.startsWith("v")){
                newVersion.remove(0,1); // remove first character
            }

            if(newVersion != GlobalVars::AppVersion){
                Util::Dialogs::showInfo
                        (
                            "There's a new version of " + GlobalVars::AppName + "! (v" + newVersion + ")<br/><br/>"+
                            "You can download it <a href='" + this->releasesUrl + "'>here</a>.", true
                            );
            }
            else{
                Util::Dialogs::showInfo("You are using last version.");
            }
        }
    }
    catch(const std::exception& e){
        QString errorMessage = "An error ocurred while checking for updates: " + QString(e.what());
        LOG_ERROR << errorMessage;
        Util::Dialogs::showError(errorMessage);
    }

    this->replyHasFinished = true;
}

// Since QNetworkReply doesn't have a way to set a timeout we need implement it by ourselves
// http://stackoverflow.com/a/13229926
void UpdateChecker::checkForQNetworkAccessManagerTimeout(QNetworkReply *reply)
{
    QTimer timer;
    timer.setSingleShot(true);

    QEventLoop loop;
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));

    timer.start(10 * 1000); // 10 seconds hardcoded timeout
    loop.exec();

    if(timer.isActive()) // request didn't timeout
    {
        timer.stop();
    }
    else if(!this->replyHasFinished)
    {
        // timeout

        // TODO this class is a bit confusing,
        // especially when we are using a boolean to check if the reply has already finished
        // (we are doing that because if user doesn't close success dialog, a second call is emitted to replyFinished function
        // after 10 secs (timeout)
        // If possible this should be refactored to a simpler method
        this->replyHasFinished = true;

        disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));

        reply->abort();

        QString errorMessage = "Timeout while checking for updates.<br/><br/>You still can check manually, by clicking <a href='" + this->releasesUrl + "'>here</a>.";
        Util::Dialogs::showError(errorMessage, true);
        LOG_ERROR << errorMessage;
    }

    reply->deleteLater();

}
