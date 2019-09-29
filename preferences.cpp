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

#include "preferences.h"
#include "ui_preferences.h"

Preferences::Preferences(QWidget *parent, ConfigFileFRequest::Settings &currentSettings) :
    QDialog(parent),
    ui(new Ui::Preferences),
    currentSettings(currentSettings)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose,true ); //destroy itself once finished.

    ui->lbProjAuthDataNote->setText(
                "<b>Note:</b> These authentications are only the ones that were saved as \"FRequest Configuration File\", "
                "that is the ones saved in your FRequest configuration (the ones saved as "
                "\"FRequest Project File\" are not shown here).<br/><br/>"
                "You can see and delete here the authentications for projects that you no longer use."
                );
    ui->lbProjAuthDataNote->adjustSize(); // to show all the text (resize label automatically to fit)

    fillConfigProjAuthDataTable();

    ui->twConfigProjAuthData->resizeColumnsToContents();
}

Preferences::~Preferences()
{
    delete ui;
}

void Preferences::showEvent(QShowEvent *e)
{
    if(!this->preferencesAreFullyLoaded)
    {
        // Apparently Qt doesn't contains a slot to when the Preferences was fully load. So we do our own implementation instead.
        connect(this, SIGNAL(signalPreferencesAreLoaded()), this, SLOT(preferencesHaveLoaded()), Qt::ConnectionType::QueuedConnection);
        emit signalPreferencesAreLoaded();
    }

    e->accept();
}

// Called only when the Preferences was fully loaded and painted on the screen. This slot is only called once.
void Preferences::preferencesHaveLoaded(){
    loadExistingSettings();

    this->preferencesAreFullyLoaded = true;
}

// Need to override to do the verification
// http://stackoverflow.com/questions/3261676/how-to-make-qdialogbuttonbox-not-close-its-parent-qdialog
void Preferences::accept (){

    if(ui->cbProxyUseProxy->isChecked()){
        if(ui->cbProxyType->currentText() != "Automatic"){

            QString proxyHostname = ui->leProxyHostname->text();
            QString proxyPortNumber = ui->leProxyPort->text();

            if(Util::Validation::checkEmptySpaces(QStringList() << proxyHostname << proxyPortNumber)){
                Util::Dialogs::showError("Please fill the proxy hostname and port number.");
                return;
            }

            if(Util::Validation::checkIfIntegers(QStringList() << proxyPortNumber)){
                Util::Dialogs::showError("Proxy port must be a number.");
                return;
            }
        }
    }

    // https://stackoverflow.com/a/16487964/1499019
    this->currentSettings.requestTimeout = QTime(0, 0, 0).secsTo(ui->teRequestTimeout->time());

    this->currentSettings.maxRequestResponseDataSizeToDisplay = ui->sbMaxRequestResponseDataSizeToDisplay->value();

    if(ui->cbOnStartup->currentText() == "Load last project"){
        this->currentSettings.onStartupSelectedOption = ConfigFileFRequest::OnStartupOption::LOAD_LAST_PROJECT;
    }
    else if(ui->cbOnStartup->currentText() == "Ask to load last project"){
        this->currentSettings.onStartupSelectedOption = ConfigFileFRequest::OnStartupOption::ASK_TO_LOAD_LAST_PROJECT;
    }
    else if(ui->cbOnStartup->currentText() == "Do nothing"){
        this->currentSettings.onStartupSelectedOption = ConfigFileFRequest::OnStartupOption::DO_NOTHING;
    }
    else{
        QString errorMessage = "Unrecognized cbOnStartup option selected! The On startup option will not be saved!";
        Util::Dialogs::showError(errorMessage);
        LOG_ERROR << errorMessage;
    }

    this->currentSettings.windowsGeometry.saveWindowsGeometryWhenExiting = ui->cbSaveWindowGeometryWhenExiting->isChecked();
    this->currentSettings.defaultHeaders.useDefaultHeaders = ui->cbUseDefaultHeaders->isChecked();

    this->currentSettings.useProxy = ui->cbProxyUseProxy->isChecked();

    bool differentThanAutomaticProxy = true;

    if(ui->cbProxyType->currentText() == "Automatic"){
        this->currentSettings.proxySettings.type = ConfigFileFRequest::ProxyType::AUTOMATIC;
        differentThanAutomaticProxy = false;
    }
    else if(ui->cbProxyType->currentText() == "Http Transparent Proxy"){
        this->currentSettings.proxySettings.type = ConfigFileFRequest::ProxyType::HTTP_TRANSPARENT;
    }
    else if(ui->cbProxyType->currentText() == "Http Proxy"){
        this->currentSettings.proxySettings.type = ConfigFileFRequest::ProxyType::HTTP;
    }
    else if(ui->cbProxyType->currentText() == "Socks5 Proxy"){
        this->currentSettings.proxySettings.type = ConfigFileFRequest::ProxyType::SOCKS5;
    }
    else{
        QString errorMessage = "Tried to save unreconized proxy type! '" + QString::number(static_cast<unsigned int>(this->currentSettings.proxySettings.type)) + "'. Please choose a valid type.";
        Util::Dialogs::showWarning(errorMessage);
        LOG_ERROR << errorMessage;
        return;
    }

    if(differentThanAutomaticProxy){
        this->currentSettings.proxySettings.hostName = ui->leProxyHostname->text();
        this->currentSettings.proxySettings.portNumber = ui->leProxyPort->text().toUInt();
    }

    updateCurrentDefaultHeaders();

    // Remove the requested config proj authentications
    for(const QString &currProjUuid : this->configProjAuthsToDelete){
        this->currentSettings.mapOfConfigAuths_UuidToConfigAuth.remove(currProjUuid);
    }

    ConfigFileFRequest::FRequestTheme newTheme = ConfigFileFRequest::geFRequestThemeByString(ui->cbTheme->currentText());
    bool giveThemeWarningToRestart = false;

    if(this->currentSettings.theme != newTheme){
        giveThemeWarningToRestart = true;
    }

    this->currentSettings.theme = newTheme;

    this->currentSettings.hideProjectSavedDialog = ui->cbHideProjectSavedDialog->isChecked();

    emit saveSettings();

    QDialog::accept();

    if(giveThemeWarningToRestart){
        Util::Dialogs::showWarning("The theme will only be applied once you restart " + GlobalVars::AppName + ".");
    }

    Util::Dialogs::showInfo("Settings saved with success!");
}

void Preferences::on_buttonBox_rejected()
{
    // nothing todo (it auto closes)
}

void Preferences::on_cbUseDefaultHeaders_toggled(bool checked)
{
    ui->gbRequest->setEnabled(checked);

    if(checked){
        on_cbRequestType_currentIndexChanged(ui->cbRequestType->currentText());
    }
}

void Preferences::on_cbRequestType_currentIndexChanged(const QString &arg1)
{
    ui->cbRequestBodyType->setEnabled(true);

    // Indicates if body can be set or not depending on the given option
    switch(UtilFRequest::getRequestTypeByString(ui->cbRequestType->currentText())){
    case UtilFRequest::RequestType::GET_OPTION:
    case UtilFRequest::RequestType::DELETE_OPTION:
    case UtilFRequest::RequestType::HEAD_OPTION:
    case UtilFRequest::RequestType::TRACE_OPTION:
    {
        ui->cbRequestBodyType->setEnabled(false);
        break;
    }
    case UtilFRequest::RequestType::POST_OPTION:
    case UtilFRequest::RequestType::PUT_OPTION:
    case UtilFRequest::RequestType::PATCH_OPTION:
    case UtilFRequest::RequestType::OPTIONS_OPTION:
    {
        ui->cbRequestBodyType->setEnabled(true);
        break;
    }
    default:
    {
        QString errorMessage = "Request type unknown: '" + arg1 + "'. Program can't proceed.";
        Util::Dialogs::showError(errorMessage);
        LOG_FATAL << errorMessage;
        exit(1);
    }
    }

    if(this->preferencesAreFullyLoaded){
        updateCurrentDefaultHeaders();
        loadCurrentDefaultHeaders();
    }

    this->previousRequestType = arg1;
}

void Preferences::on_tbRequestBodyKeyValueAdd_clicked()
{
    Util::TableWidget::addRow(ui->twRequestBodyKeyValue, QStringList() << "" << "");
}

void Preferences::on_tbRequestBodyKeyValueRemove_clicked()
{
    int size = Util::TableWidget::getSelectedRows(ui->twRequestBodyKeyValue).size();

    if(size==0){
        Util::Dialogs::showInfo("Select a row first!");
        return;
    }

    if(Util::Dialogs::showQuestion(this, "Are you sure you want to remove all selected rows?")){
        for(int i=0; i < size; i++){
            ui->twRequestBodyKeyValue->removeRow(Util::TableWidget::getSelectedRows(ui->twRequestBodyKeyValue).at(size-i-1).row());
        }

        Util::Dialogs::showInfo("Key-Value rows deleted");
    }
}

void Preferences::loadExistingSettings(){
    ui->teRequestTimeout->setTime(QTime(0,0).addSecs(this->currentSettings.requestTimeout));
    ui->sbMaxRequestResponseDataSizeToDisplay->setValue(this->currentSettings.maxRequestResponseDataSizeToDisplay);
    ui->cbUseDefaultHeaders->setChecked(this->currentSettings.defaultHeaders.useDefaultHeaders);
    ui->cbSaveWindowGeometryWhenExiting->setChecked(this->currentSettings.windowsGeometry.saveWindowsGeometryWhenExiting);
    ui->cbProxyUseProxy->setChecked(this->currentSettings.useProxy);
    ui->cbHideProjectSavedDialog->setChecked(this->currentSettings.hideProjectSavedDialog);

    // TODO Check if there's a better alternative to do this switches / ifs (maybe using enums??)
    // (without do direct string comparisons), because as it is, it is easy to break if we change any of the strings
    switch(this->currentSettings.onStartupSelectedOption){
    case ConfigFileFRequest::OnStartupOption::LOAD_LAST_PROJECT:
    {
        ui->cbOnStartup->setCurrentText("Load last project");
        break;
    }
    case ConfigFileFRequest::OnStartupOption::ASK_TO_LOAD_LAST_PROJECT:
    {
        ui->cbOnStartup->setCurrentText("Ask to load last project");
        break;
    }
    case ConfigFileFRequest::OnStartupOption::DO_NOTHING:
    {
        ui->cbOnStartup->setCurrentText("Do nothing");
        break;
    }
    default:
    {
        ui->cbProxyType->setCurrentText("Ask to load last project");
        QString warningMessage = "Unknown on startup option loaded! '" +
                QString::number(static_cast<unsigned int>(this->currentSettings.onStartupSelectedOption)) + "'. Using 'Ask to load last project' instead.";
        Util::Dialogs::showWarning(warningMessage);
        LOG_WARNING << warningMessage;
    }
    }

    switch(this->currentSettings.proxySettings.type){
    case ConfigFileFRequest::ProxyType::AUTOMATIC:
    {
        ui->cbProxyType->setCurrentText("Automatic");
        break;
    }
    case ConfigFileFRequest::ProxyType::HTTP_TRANSPARENT:
    {
        ui->cbProxyType->setCurrentText("Http Transparent Proxy");
        break;
    }
    case ConfigFileFRequest::ProxyType::HTTP:
    {
        ui->cbProxyType->setCurrentText("Http Proxy");
        break;
    }
    case ConfigFileFRequest::ProxyType::SOCKS5:
    {
        ui->cbProxyType->setCurrentText("Socks5 Proxy");
        break;
    }
    default:
    {
        ui->cbProxyType->setCurrentText("Automatic");
        QString warningMessage = "Unknown proxy type loaded! '" + QString::number(static_cast<unsigned int>(this->currentSettings.proxySettings.type)) + "'. Using 'automatic' instead.";
        Util::Dialogs::showWarning(warningMessage);
        LOG_WARNING << warningMessage;
    }
    }

    ui->leProxyHostname->setText(this->currentSettings.proxySettings.hostName);
    ui->leProxyPort->setText(QString::number(this->currentSettings.proxySettings.portNumber));

    loadCurrentDefaultHeaders();

    ui->cbTheme->setCurrentText(ConfigFileFRequest::getFRequestThemeString(this->currentSettings.theme));
}

void Preferences::loadCurrentDefaultHeaders(){

    Util::TableWidget::clearContentsNoPrompt(ui->twRequestBodyKeyValue);

    UtilFRequest::RequestType currentRequestType = UtilFRequest::getRequestTypeByString(ui->cbRequestType->currentText());

    std::experimental::optional<ConfigFileFRequest::ProtocolHeader> &currentProtocolHeader = ConfigFileFRequest::getSettingsHeaderForRequestType(currentRequestType, this->currentSettings);
    bool mayHaveBody = UtilFRequest::requestTypeMayHaveBody(currentRequestType);

    if(!mayHaveBody){
        if(currentProtocolHeader.has_value()){
            if(currentProtocolHeader.value().headers_Raw.has_value()){
                for(const UtilFRequest::HttpHeader &currentHeader : currentProtocolHeader.value().headers_Raw.value())
                {
                    Util::TableWidget::addRow(ui->twRequestBodyKeyValue, QStringList() << currentHeader.name << currentHeader.value);
                }
            }
        }
    }
    else{
        if(currentProtocolHeader.has_value()){

            std::experimental::optional<QVector<UtilFRequest::HttpHeader>> *currentProtocolHeaders = nullptr;

            if(ui->cbRequestBodyType->currentText() == "raw"){
                currentProtocolHeaders = &currentProtocolHeader.value().headers_Raw;
            }
            else if(ui->cbRequestBodyType->currentText() == "form-data"){
                currentProtocolHeaders = &currentProtocolHeader.value().headers_Form_Data;
            }
            else if(ui->cbRequestBodyType->currentText() == "x-form-www-urlencoded"){
                currentProtocolHeaders = &currentProtocolHeader.value().headers_X_form_www_urlencoded;
            }
            else{
                QString errorMessage = "Error loading default headers! The type '" + ui->cbRequestBodyType->currentText() + "' is not being handled!";
                LOG_ERROR << errorMessage;
                Util::Dialogs::showError(errorMessage);
                return;
            }

            if(currentProtocolHeaders->has_value()){
                for(const UtilFRequest::HttpHeader &currentHeader : currentProtocolHeaders->value())
                {
                    Util::TableWidget::addRow(ui->twRequestBodyKeyValue, QStringList() << currentHeader.name << currentHeader.value);
                }
            }
        }
    }
}

std::experimental::optional<QVector<UtilFRequest::HttpHeader>> Preferences::getRequestHeaders(){

    std::experimental::optional<QVector<UtilFRequest::HttpHeader>> requestHeaders;

    if(ui->twRequestBodyKeyValue->rowCount() > 0){
        requestHeaders = QVector<UtilFRequest::HttpHeader>();
    }

    for(int i = 0; i < ui->twRequestBodyKeyValue->rowCount(); i++){

        UtilFRequest::HttpHeader currentHeader;

        currentHeader.name = ui->twRequestBodyKeyValue->item(i, 0)->text();
        currentHeader.value = ui->twRequestBodyKeyValue->item(i, 1)->text();

        requestHeaders.value().append(currentHeader);
    }

    return requestHeaders;
}

void Preferences::on_cbRequestBodyType_currentIndexChanged(const QString &arg1)
{
    if(this->preferencesAreFullyLoaded){
        updateCurrentDefaultHeaders();
        loadCurrentDefaultHeaders();
    }

    this->previousRequestBodyType = arg1;
}

void Preferences::updateCurrentDefaultHeaders(){
    std::experimental::optional<QVector<UtilFRequest::HttpHeader>> currentRequestDefaultHeaders = getRequestHeaders();
    std::experimental::optional<QVector<UtilFRequest::HttpHeader>> *currentHeaders = nullptr;

    UtilFRequest::RequestType requestType = UtilFRequest::getRequestTypeByString(this->previousRequestType);

    std::experimental::optional<ConfigFileFRequest::ProtocolHeader> &currentProtocolHeader = ConfigFileFRequest::getSettingsHeaderForRequestType(requestType, this->currentSettings);
    bool mayHaveBody = UtilFRequest::requestTypeMayHaveBody(requestType);

    if(!mayHaveBody){
        currentProtocolHeader = ConfigFileFRequest::ProtocolHeader();

        currentHeaders = &currentProtocolHeader.value().headers_Raw;
    }
    else{
        // Create if not exist
        if(!currentProtocolHeader.has_value()){
            currentProtocolHeader = ConfigFileFRequest::ProtocolHeader();
        }

        if(this->previousRequestBodyType == "raw"){
            currentHeaders = &currentProtocolHeader.value().headers_Raw;
        }
        else if(this->previousRequestBodyType == "form-data"){
            currentHeaders = &currentProtocolHeader.value().headers_Form_Data;
        }
        else if(this->previousRequestBodyType == "x-form-www-urlencoded"){
            currentHeaders = &currentProtocolHeader.value().headers_X_form_www_urlencoded;
        }
        else{
            QString errorMessage = "Error loading default headers! The type '" + ui->cbRequestBodyType->currentText() + "' is not being handled!";
            LOG_ERROR << errorMessage;
            Util::Dialogs::showError(errorMessage);
            return;
        }
    }

    (*currentHeaders) = currentRequestDefaultHeaders;
}

void Preferences::on_cbProxyUseProxy_toggled(bool checked)
{
    ui->gbProxy->setEnabled(checked);
}

void Preferences::on_cbProxyType_currentIndexChanged(const QString &arg1)
{
    if(arg1 == "Automatic"){
        ui->leProxyHostname->setEnabled(false);
        ui->leProxyPort->setEnabled(false);
    }
    else{
        ui->leProxyHostname->setEnabled(true);
        ui->leProxyPort->setEnabled(true);
    }
}

void Preferences::fillConfigProjAuthDataTable(){
    for(const ConfigFileFRequest::ConfigurationProjectAuthentication &currAuthData :
        this->currentSettings.mapOfConfigAuths_UuidToConfigAuth){
        Util::TableWidget::addRow(ui->twConfigProjAuthData, QStringList() <<
                                  currAuthData.lastProjectName <<
                                  currAuthData.projectUuid <<
                                  FRequestAuthentication::getAuthenticationString(currAuthData.authData->type)
                                  );
    }

    // Disable editing on our table
    for(int i=0; i<ui->twConfigProjAuthData->rowCount(); i++){
        for(int j=0; j<ui->twConfigProjAuthData->columnCount(); j++){
            ui->twConfigProjAuthData->item(i,j)->setFlags(ui->twConfigProjAuthData->item(i,j)->flags() & (~Qt::ItemIsEditable));
        }
    }
}

void Preferences::on_tbConfigProjDataRemove_clicked()
{
    int size = Util::TableWidget::getSelectedRows(ui->twConfigProjAuthData).size();

    if(size==0){
        Util::Dialogs::showInfo("Select a row first!");
        return;
    }

    if(Util::Dialogs::showQuestion(this, "Are you sure you want to remove all selected rows?")){
        for(int i=0; i < size; i++){
            this->configProjAuthsToDelete.append(ui->twConfigProjAuthData->item(Util::TableWidget::getSelectedRows(ui->twConfigProjAuthData).at(size-i-1).row(),1)->text()); // TODO: replace 1 by an index enum
            ui->twConfigProjAuthData->removeRow(Util::TableWidget::getSelectedRows(ui->twConfigProjAuthData).at(size-i-1).row());
        }

        Util::Dialogs::showInfo("Authentications rows deleted");
    }
}
