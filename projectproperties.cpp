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

#include "projectproperties.h"
#include "ui_projectproperties.h"

ProjectProperties::ProjectProperties(QWidget *parent, FRequestTreeWidgetProjectItem * const projectItem) :
    QDialog(parent),
    ui(new Ui::ProjectProperties),
    projectItem(projectItem)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose,true); //destroy itself once finished.
    fillInterface();

    if(this->projectItem->authData != nullptr){
        fillAuthenticationData(*this->projectItem->authData);
    }

    if(this->currentPasswordSalt.isEmpty()){
        // Unix time as sha256
        this->currentPasswordSalt = QCryptographicHash::hash(QByteArray().setNum(QDateTime::currentDateTime().toSecsSinceEpoch()), QCryptographicHash::Sha256).toHex();
    }

    if(ui->cbRequestType->currentText() == "Request Authentication"){
        ui->saProjectPropertiesNote->setVisible(true);
    }
    else{
        ui->saProjectPropertiesNote->setVisible(false);
    }

    ui->lbProjectPropertiesNote->setText(
                "<b>Note:</b> Request authentication works by selecting one of your requests as the one for the authentication.<br/><br/>"
                "If you have a website you can find the request that you need by checking in your browser the one that authenticates you "
                "(normally you could find it in a \"network\" tab), "
                "then just replicate that request in FRequest and select it for the authentication here.<br/><br/>"
                "Currently FRequest can only use a single request for the authentication.<br/><br/>"
                "You should add in your authentication request the FRequest placeholders for the username and password, they are respectively "
                "{{FREQUEST_AUTH_USERNAME}} and {{FREQUEST_AUTH_PASSWORD}}.<br/><br/>"
                "You can add these placeholders either in the body of the request or in the headers. "
                "FRequest will replace them automatically when authenticating by the username and password that you input above."
                );
}

void ProjectProperties::fillInterface(){

    ui->leProjectName->setText(this->projectItem->projectName);
    ui->leProjectMainUrl->setText(this->projectItem->projectMainUrl);

    // Get all requests name to fill request combobox
    for(int i=0; i < this->projectItem->childCount(); i++){
        FRequestTreeWidgetRequestItem* currentRequest = FRequestTreeWidgetRequestItem::fromQTreeWidgetItem(this->projectItem->child(i));

        ui->cbRequestForAuthentication->addItem(getComboBoxNameForRequest(currentRequest), QVariant::fromValue(currentRequest));
    }

    ui->cbIdentCharacter->setCurrentText(UtilFRequest::getIdentCharacterString(this->projectItem->saveIdentCharacter));

    for (int i = 0; i < this->projectItem->globalHeaders.size(); i++) {
        Util::TableWidget::addRow(
            ui->headerKeyValue,
            QStringList() << this->projectItem->globalHeaders.at(i).name << this->projectItem->globalHeaders.at(i).value);
    }
}

ProjectProperties::~ProjectProperties()
{
    delete ui;
}

void ProjectProperties::on_cbRequestType_currentIndexChanged(const QString &arg1)
{
    ui->cbRequestForAuthentication->setEnabled(false);

    if(arg1 == "Request Authentication"){
        ui->cbRequestForAuthentication->setEnabled(true);
    }
}

// Need to override to do the verification
// http://stackoverflow.com/questions/3261676/how-to-make-qdialogbuttonbox-not-close-its-parent-qdialog
void ProjectProperties::accept (){

    // Validations
    if(Util::Validation::checkEmptySpaces(
                QStringList() <<
                ui->leProjectName->text() <<
                ui->leProjectMainUrl->text()
                )){
        Util::Dialogs::showError("Please fill the project name and main url.");
        return;
    }

    if(ui->cbUseAuthentication->isChecked() && Util::Validation::checkEmptySpaces(
                QStringList() <<
                ui->leUsername->text()
                )){
        Util::Dialogs::showError("Please fill the authorization username and authorization password.");
        return;
    }

    this->projectItem->projectName = ui->leProjectName->text();
    this->projectItem->setText(0, this->projectItem->projectName); // todo replace column with enum
    this->projectItem->projectMainUrl = ui->leProjectMainUrl->text();

    if(ui->cbUseAuthentication->isChecked()){
        switch(FRequestAuthentication::getAuthenticationTypeByString(ui->cbRequestType->currentText())){
        case FRequestAuthentication::AuthenticationType::REQUEST_AUTHENTICATION:
        {
            this->projectItem->authData = std::make_shared<RequestAuthentication>(
                        ui->rbSaveProjectAuthToConfigurationFile->isChecked(),
                        ui->cbRetryLoginIfError401->isChecked(),
                        ui->leUsername->text(),
                        this->currentPasswordSalt,
                        ui->lePassword->text(),
                        qvariant_cast<FRequestTreeWidgetRequestItem*>(ui->cbRequestForAuthentication->itemData(ui->cbRequestForAuthentication->currentIndex()))->itemContent.uuid
                        );
            break;
        }
        case FRequestAuthentication::AuthenticationType::BASIC_AUTHENTICATION:
        {
            this->projectItem->authData = std::make_shared<BasicAuthentication>(BasicAuthentication(ui->rbSaveProjectAuthToConfigurationFile->isChecked(),
                                                                                                    ui->cbRetryLoginIfError401->isChecked(),
                                                                                                    ui->leUsername->text(),
                                                                                                    this->currentPasswordSalt,
                                                                                                    ui->lePassword->text()
                                                                                                    ));
            break;
        }
        default:
        {
            QString errorMessage = "Authentication type unknown: '" + ui->cbRequestType->currentText() + "'. Program can't proceed.";
            Util::Dialogs::showError(errorMessage);
            LOG_FATAL << errorMessage;
            exit(1);
        }
        }
    }
    else{
        this->projectItem->authData = nullptr; // clear the old authentication data
    }

    UtilFRequest::IdentCharacter identCharacter = UtilFRequest::getIdentCharacterByString(ui->cbIdentCharacter->currentText());

    switch(identCharacter){
    case UtilFRequest::IdentCharacter::SPACE:
    case UtilFRequest::IdentCharacter::TAB:
    {
        this->projectItem->saveIdentCharacter = identCharacter;
        break;
    }
    default:
    {
        QString errorMessage = "Ident character unknown: '" + ui->cbIdentCharacter->currentText() + "'. Program can't proceed.";
        Util::Dialogs::showError(errorMessage);
        LOG_FATAL << errorMessage;
        exit(1);
    }
    }

    this->projectItem->globalHeaders.clear();
    for (int i = 0; i < ui->headerKeyValue->rowCount(); i++) {
        UtilFRequest::HttpHeader h = {ui->headerKeyValue->item(i, 0)->text(), ui->headerKeyValue->item(i, 1)->text()};
        this->projectItem->globalHeaders.append(h);
    }

    QDialog::accept();

    emit signalSaveProjectProperties();
}

void ProjectProperties::on_cbUseAuthentication_toggled(bool checked)
{
    ui->gbAuthentication->setEnabled(checked);
}

void ProjectProperties::on_headerKeyValueAdd_clicked()
{
    Util::TableWidget::addRow(ui->headerKeyValue, QStringList() << "" << "");
}

void ProjectProperties::on_headerKeyValueRemove_clicked()
{

    int size = Util::TableWidget::getSelectedRows(ui->headerKeyValue).size();

    if(size==0){
        Util::Dialogs::showInfo("Select a row first!");
        return;
    }

    if(Util::Dialogs::showQuestion(this, "Are you sure you want to remove all selected rows?")){
        for(int i=0; i < size; i++){
            ui->headerKeyValue->removeRow(Util::TableWidget::getSelectedRows(ui->headerKeyValue).at(size-i-1).row());
        }
        Util::Dialogs::showInfo("Key-Value rows deleted");
    }
}

void ProjectProperties::fillAuthenticationData(FRequestAuthentication &auth){

    ui->cbUseAuthentication->setChecked(true);

    if(auth.saveAuthToConfigFile){
        ui->rbSaveProjectAuthToConfigurationFile->setChecked(true);
    }
    else{
        ui->rbSaveProjectAuthToProjectFile->setChecked(true);
    }

    ui->cbRequestType->setCurrentText(FRequestAuthentication::getAuthenticationString(auth.type));

    switch(auth.type){
    case FRequestAuthentication::AuthenticationType::REQUEST_AUTHENTICATION:
    {
        // https://stackoverflow.com/a/43572369/1499019
        RequestAuthentication &concreteAuth = static_cast<RequestAuthentication&>(auth);

        ui->leUsername->setText(concreteAuth.username);
        ui->lePassword->setText(concreteAuth.password);
        ui->cbRequestForAuthentication->setCurrentIndex(
                    ui->cbRequestForAuthentication->findData(QVariant::fromValue(this->projectItem->getChildRequestByUuid(concreteAuth.requestForAuthenticationUuid)))
                    );
        this->currentPasswordSalt = concreteAuth.passwordSalt;
        break;
    }
    case FRequestAuthentication::AuthenticationType::BASIC_AUTHENTICATION:
    {
        BasicAuthentication &concreteAuth = static_cast<BasicAuthentication&>(auth);

        ui->leUsername->setText(concreteAuth.username);
        ui->lePassword->setText(concreteAuth.password);
        this->currentPasswordSalt = concreteAuth.passwordSalt;
        break;
    }
    default:
    {
        QString errorMessage = "Invalid authentication type " + QString::number(static_cast<int>(auth.type)) + "'. Program can't proceed.";
        Util::Dialogs::showError(errorMessage);
        LOG_FATAL << errorMessage;
        exit(1);
    }
    }

    ui->cbRetryLoginIfError401->setChecked(auth.retryLoginIfError401);
}

QString ProjectProperties::getComboBoxNameForRequest(const FRequestTreeWidgetRequestItem* const currentRequest){
    return currentRequest->itemContent.name + " (" + UtilFRequest::getRequestTypeString(currentRequest->itemContent.requestType) + ")";
}
