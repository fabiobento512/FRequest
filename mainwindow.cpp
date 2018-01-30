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

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // We use this appender because it is the native way to have \r\n in windows in plog library
    // example: https://github.com/SergiusTheBest/plog/blob/master/samples/NativeEOL/Main.cpp
    static plog::RollingFileAppender<plog::TxtFormatter, plog::NativeEOLConverter<>> fileAppender
            (QSTR_TO_CSTR(Util::FileSystem::getAppPath() + "/" + GlobalVars::AppLogFileName), 1024*5 /* 5 Mb max log size */, 3);
    plog::init(plog::info, &fileAppender);

    this->currentSettings = this->configFileManager.getCurrentSettings();

    this->ignoreAnyChangesToProject.SetCondition();
    ui->setupUi(this);

    // We aren't using this yet, so close it
    ui->mainToolBar->close();

    // show request types icons or not?
    ui->actionShow_Request_Types_Icons->setChecked(this->currentSettings.showRequestTypesIcons);

    // Disable drop of items outside of the project folder
    // Thanks to "p.i.g.": http://stackoverflow.com/a/30580654
    ui->treeWidget->invisibleRootItem()->setFlags(
                ui->treeWidget->invisibleRootItem()->flags() ^ Qt::ItemIsDropEnabled
                );

    // Set max icon size in tree widget
    ui->treeWidget->setIconSize(QSize(32,16));

    setNewProject();
    // Set our desired proportion for the projects tree widget and remaining interface
    // this->width() expands the second widget as much as possible
    ui->splitter->setSizes(QList<int>() << ui->treeWidget->minimumWidth() << this->width());

    QPalette currentPalette;
    currentPalette.setColor(QPalette::Active, QPalette::Base, currentPalette.color(QPalette::Disabled, QPalette::Base));
    ui->leFullPath->setPalette(currentPalette); // Set the background color the same as disable

#ifdef Q_OS_MAC
    ui->pbSendRequest->setToolTip(ui->pbSendRequest->toolTip() + " (⌘ + Enter)");
#else
    ui->pbSendRequest->setToolTip(ui->pbSendRequest->toolTip() + " (Ctrl + Enter)");
#endif

    loadRecentProjects();

    // Hide response warning messages and icons for now
    ui->lbResponseBodyWarningIcon->hide();
    ui->lbResponseBodyWarningMessage->hide();
    ui->lbResponseBodyWarningIcon->setMaximumSize(0,0);
    ui->lbResponseBodyWarningMessage->setMaximumSize(0,0);
    ui->lbResponseBodyWarningMessage->setToolTip(ui->lbResponseBodyWarningMessage->text());

    // Hide key value table for now (the raw textedit is displayed by default)
    ui->twRequestBodyKeyValue->setMaximumSize(0,0);
    ui->tbRequestBodyKeyValueAdd->setMaximumSize(0,0);
    ui->tbRequestBodyFile->setMaximumSize(0,0);
    ui->tbRequestBodyKeyValueRemove->setMaximumSize(0,0);

    ui->twRequestBodyKeyValue->hide();
    ui->tbRequestBodyKeyValueAdd->hide();
    ui->tbRequestBodyFile->hide();
    ui->tbRequestBodyKeyValueRemove->hide();

    // We will also set their size to the normal size (we start with size 0 so vertical splitter divide the area in equal space (for header and response), kinda hacky)
    ui->twRequestBodyKeyValue->setMaximumSize(this->auxMaximumSize);
    ui->tbRequestBodyKeyValueAdd->setMaximumSize(this->auxMaximumSize);
    ui->tbRequestBodyFile->setMaximumSize(this->auxMaximumSize);
    ui->tbRequestBodyKeyValueRemove->setMaximumSize(this->auxMaximumSize);
    ui->lbResponseBodyWarningIcon->setMaximumSize(16, 16);
    ui->lbResponseBodyWarningMessage->setMaximumSize(this->auxMaximumSize);

    // Scretch middle column (value one) for form key value request body
    QHeaderView *twRequestBodyKeyValueHeaderView = ui->twRequestBodyKeyValue->horizontalHeader();
    twRequestBodyKeyValueHeaderView->setSectionResizeMode(1, QHeaderView::Stretch);

    // Progress indicator and abort button (for the request being made)
    this->pbRequestProgress.setTextVisible(false); //hides text
    this->pbRequestProgress.setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Fixed);
    this->pbRequestProgress.setMinimumWidth(150);
    this->pbRequestProgress.setMaximum(0);
    ui->statusBar->addWidget(&this->lbProjectInfo);
    ui->statusBar->addPermanentWidget(&this->lbRequestInfo);
    ui->statusBar->addPermanentWidget(&this->pbRequestProgress); //this adds automatically in right
    this->tbAbortRequest.setIcon(QIcon(":/icons/abort.png"));
    this->tbAbortRequest.setAutoRaise(true);
    this->tbAbortRequest.setToolTip("Abort current request");
    connect(&this->tbAbortRequest , SIGNAL (clicked()), this, SLOT(tbAbortRequest_clicked())); // connect button click to our slot function
    connect(&this->networkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
	connect(ui->treeWidget, SIGNAL(deleteKeyPressed()), this, SLOT(treeWidgetDeleteKeyPressed()));
    ui->statusBar->addPermanentWidget(&this->tbAbortRequest);
    this->lbRequestInfo.hide();
    this->pbRequestProgress.hide();
    this->tbAbortRequest.hide();

    // Restore geometry if it exists
    if(this->currentSettings.windowsGeometry.saveWindowsGeometryWhenExiting){
        if(!this->currentSettings.windowsGeometry.mainWindow_MainWindowGeometry.isEmpty()){
            if(!this->restoreGeometry(this->currentSettings.windowsGeometry.mainWindow_MainWindowGeometry)){
                QString errorMessage = "Couldn't restore saved main window geometry.";
                Util::Dialogs::showError(errorMessage);
                LOG_ERROR << errorMessage;
                return;
            }

            if(!ui->splitter->restoreState(this->currentSettings.windowsGeometry.mainWindow_RequestsSplitterState)){
                QString errorMessage = "Couldn't restore saved requests splitter state.";
                Util::Dialogs::showError(errorMessage);
                LOG_ERROR << errorMessage;
                return;
            }

            if(!ui->splitter_2->restoreState(this->currentSettings.windowsGeometry.mainWindow_RequestResponseSplitterState)){
                QString errorMessage = "Couldn't restore saved request response splitter state.";
                Util::Dialogs::showError(errorMessage);
                LOG_ERROR << errorMessage;
                return;
            }
        }
    }
    else{
        // center window if we are not restoring geometry
        this->setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter,this->size(),qApp->desktop()->availableGeometry()));
    }

    ui->treeWidget->setFocus();
}

void MainWindow::showEvent(QShowEvent *e)
{
    if(!this->applicationIsFullyLoaded)
    {
        // Apparently Qt doesn't contains a slot to when the application was fully load (mainwindow). So we do our own implementation instead.
        connect(this, SIGNAL(signalAppIsLoaded()), this, SLOT(applicationHasLoaded()), Qt::ConnectionType::QueuedConnection);
        emit signalAppIsLoaded();
    }

    e->accept();
}

// Called only when the MainWindow was fully loaded and painted on the screen. This slot is only called once.
void MainWindow::applicationHasLoaded(){

    LOG_INFO << GlobalVars::AppName + " " + GlobalVars::AppVersion + " started";

    this->applicationIsFullyLoaded = true;
    this->ignoreAnyChangesToProject.UnsetCondition();

    if(this->currentSettings.askToOpenLastProject && this->currentSettings.recentProjectsPaths.size() > 0){
        QString lastSavedProject = this->currentSettings.recentProjectsPaths[0];

        if(!lastSavedProject.isEmpty()){
            if(Util::Dialogs::showQuestion(this,"Do you want to load latest project?\n\nLatest project was '" + Util::FileSystem::cutNameWithoutBackSlash(lastSavedProject) + "'.")){
                loadProjectState(lastSavedProject);
            }
        }
    }
}


MainWindow::~MainWindow()
{
    delete ui;
    LOG_INFO << GlobalVars::AppName + " " + GlobalVars::AppVersion + " exited";
}

void MainWindow::on_pbSendRequest_clicked()
{
	
    if(formKeyValueInBodyIsValid()){
		
		 // Disable until this request is finished
        ui->pbSendRequest->setEnabled(false);
        this->lbRequestInfo.show();
        this->pbRequestProgress.show();
        this->tbAbortRequest.show();
        this->pbRequestProgress.setValue(0);
		ui->gbProject->setEnabled(false);
        ui->gbRequest->setEnabled(false);
		
		// Apply proxy type
		if(this->currentSettings.useProxy){
            QNetworkProxy proxy;

            switch (this->currentSettings.proxySettings.type) {
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
                QString errorMessage = "Unknown proxy type set! '" + QString::number(static_cast<unsigned int>(this->currentSettings.proxySettings.type)) + "'. Check the proxy settings.";
                Util::Dialogs::showError(errorMessage);
                LOG_ERROR << errorMessage;
                Util::StatusBar::showError(ui->statusBar, errorMessage);
                return;
            }
            }

            if(this->currentSettings.proxySettings.type != ConfigFileFRequest::ProxyType::AUTOMATIC){
                QNetworkProxyFactory::setUseSystemConfiguration(false);
                proxy.setHostName(this->currentSettings.proxySettings.hostName);
                proxy.setPort(this->currentSettings.proxySettings.portNumber);
                QNetworkProxy::setApplicationProxy(proxy);
                this->networkAccessManager.setProxy(proxy);
            }
        }
        else{
            QNetworkProxy proxy;
            proxy.setType(QNetworkProxy::NoProxy);
            this->networkAccessManager.setProxy(proxy);
        }
		
		QVector<UtilFRequest::HttpHeader> requestFinalHeaders = getRequestHeaders();
		
		// Attempt to authenticate if we have authentication and its the first request in this project
        if(this->currentProjectItem->authData != nullptr){
				
			// We only apply the authentication to urls of the project, overriden urls don't have the auth applied
			if(!ui->cbRequestOverrideMainUrl->isChecked()){
				switch(this->currentProjectItem->authData->type){
					case FRequestAuthentication::AuthenticationType::REQUEST_AUTHENTICATION:
					{
						if(!this->currentProjectAuthenticationWasMade)
						{
							this->lbRequestInfo.setText("Authenticating using the request provided...");

							applyRequestAuthentication();
							
							// If there was an error with the authorization don't proceed
							if(!this->currentProjectAuthenticationWasMade){
								return;
							}
						}
						break;
					}
					case FRequestAuthentication::AuthenticationType::BASIC_AUTHENTICATION:
					{
						const RequestAuthentication * const concreteAuthData = static_cast<RequestAuthentication*>(this->currentProjectItem->authData.get());
						
						UtilFRequest::HttpHeader currentHeader;

						currentHeader.name = "Authorization";
						currentHeader.value = "Basic " + QString(concreteAuthData->username + ":" + concreteAuthData->password).toUtf8().toBase64();
						
						requestFinalHeaders.append(currentHeader);
						
						break;
					}
					default:
					{
						QString errorMessage = "Invalid authentication type " + QString::number(static_cast<int>(this->currentProjectItem->authData->type)) + "'. Program can't proceed.";
						Util::Dialogs::showError(errorMessage);
						LOG_FATAL << errorMessage;
						exit(1);
					}
				}	
			}
		}
		
        this->ignoreAnyChangesToProject.SetCondition();
        formatRequestBody(getRequestCurrentSerializationFormatType());
        this->ignoreAnyChangesToProject.UnsetCondition();

        // Display info about when request was started
        this->lbRequestInfo.setText("Requested started at " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

        // Clear previous request data:
        clearOlderResponse();
		
        this->currentReply = processHttpRequest
                (
                    UtilFRequest::getRequestTypeByString(ui->cbRequestType->currentText()),
                    ui->leFullPath->text(),
                    ui->cbBodyType->currentText(),
                    ui->pteRequestBody->toPlainText(),
                    requestFinalHeaders
                    );

        lastStartTime = QDateTime::currentDateTime();

        checkForQNetworkAccessManagerTimeout(this->currentReply.value());
    }
}

void MainWindow::applyRequestAuthentication(){
	
	std::shared_ptr<FRequestAuthentication> authData = this->currentProjectItem->authData;


	if(authData->type != FRequestAuthentication::AuthenticationType::REQUEST_AUTHENTICATION)
	{
		QString errorMessage = "Authentication is not a REQUEST_AUTHENTICATION. Please report this error.";
		Util::Dialogs::showError(errorMessage);
		LOG_ERROR << errorMessage;
		return;
	}
	
	this->authenticationIsRunning = true;
	
	const RequestAuthentication * const concreteAuthData = static_cast<RequestAuthentication*>(authData.get());

	const FRequestTreeWidgetRequestItem * const authRequestItem =
			this->currentProjectItem->getChildRequestByUuid(concreteAuthData->requestForAuthenticationUuid);

	QString mainUrl;

	if(authRequestItem->itemContent.bOverridesMainUrl){
		mainUrl = authRequestItem->itemContent.overrideMainUrl;
	}
	else{
		mainUrl = this->currentProjectItem->projectMainUrl;
	}

	// Wait for authentication request synchronously
	// https://stackoverflow.com/a/5496468/1499019
	QEventLoop loop;
	connect(this, SIGNAL(signalRequestFinishedAndProcessed()), &loop, SLOT(quit()));
	
	QVector<UtilFRequest::HttpHeader> finalAuthRequestHeaders = authRequestItem->itemContent.headers;
	
	// Apply auth data to headers if it the placeholder fields are found
	for(UtilFRequest::HttpHeader &currentHeader : finalAuthRequestHeaders){
		currentHeader.name = UtilFRequest::replaceFRequestAuthenticationPlaceholders(currentHeader.name, concreteAuthData->username, concreteAuthData->password);
        currentHeader.value = UtilFRequest::replaceFRequestAuthenticationPlaceholders(currentHeader.value, concreteAuthData->username, concreteAuthData->password);
	}
	
	this->currentReply = processHttpRequest(
				authRequestItem->itemContent.requestType,
				getFullPathFromMainUrlAndPath(mainUrl, UtilFRequest::replaceFRequestAuthenticationPlaceholders(authRequestItem->itemContent.path, concreteAuthData->username, concreteAuthData->password)),
				UtilFRequest::getBodyTypeString(authRequestItem->itemContent.bodyType),
				UtilFRequest::replaceFRequestAuthenticationPlaceholders(authRequestItem->itemContent.body, concreteAuthData->username, concreteAuthData->password),
				finalAuthRequestHeaders
                );
				
	// checkForQNetworkAccessManagerTimeout(this->currentReply.value()); // TODO: check why this is causing problems
				
	loop.exec();
}

// Since QNetworkReply doesn't have a way to set a timeout we need implement it by ourselves
// http://stackoverflow.com/a/13229926
void MainWindow::checkForQNetworkAccessManagerTimeout(QNetworkReply *reply)
{
    QTimer timer;
    timer.setSingleShot(true);

    QEventLoop loop;
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));

    timer.start(this->currentSettings.requestTimeout * 1000);
    loop.exec();

    if(timer.isActive() || timer.interval() == 0) // if request timeout is 0 don't timeout
    {
        timer.stop();
    }
    else
    {
        // timeout
		
		QString errorMessage = "Timeout after " + QString::number(this->currentSettings.requestTimeout) + " seconds";
		
		if(this->authenticationIsRunning){
			Util::Dialogs::showError(errorMessage);
		}
		else{
			ui->lbStatus->setText("-1");
			ui->lbDescription->setText(errorMessage);
			ui->lbTimeElapsed->setText(QString::number(lastStartTime.msecsTo(QDateTime::currentDateTime())) + " ms");
		}
		
		LOG_ERROR << errorMessage;

        this->lastReplyStatusError = -1;

        disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));

        reply->abort();
    }
}

void MainWindow::replyFinished(QNetworkReply *reply){

	QString requestReturnCode;
	QString requestReturnMessage;
	bool isError = false;
	bool isToRetryWithAuthentication = false;
	
	// -1 means we have set a custom error, we don't want to override that one
    if(this->lastReplyStatusError != -1 && reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).isValid()){ // success request
		requestReturnCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
		requestReturnMessage = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    }
    else if(this->lastReplyStatusError == 0 && reply->error() != QNetworkReply::NoError){
		requestReturnCode = "N/A";
		requestReturnMessage = reply->errorString() + " - Error " + QString::number(reply->error());
		isError = true;
    }
	
	if(!this->authenticationIsRunning){
		ui->lbStatus->setText(requestReturnCode);
		ui->lbDescription->setText(requestReturnMessage);
	}

	if(!this->authenticationIsRunning){
		ui->lbTimeElapsed->setText(QString::number(lastStartTime.msecsTo(QDateTime::currentDateTime())) + " ms");
	}
	
    if(reply->error() == QNetworkReply::NoError)
    {
		if(!this->authenticationIsRunning){
			// TODO: Add an option for this and clean up this code
			// If greater than 200 KB only give the option to download the file
			constexpr int maxBytesForBufferAndDisplay = 200*1024;
			QString headersText;
			QByteArray totalLoadedData;
			QByteArray currentData;

			// Read the bytes to display
			totalLoadedData = reply->read(maxBytesForBufferAndDisplay);

			for(const QNetworkReply::RawHeaderPair &currentPair : reply->rawHeaderPairs()){
				headersText += currentPair.first + ": " + currentPair.second + "\n";
			}

			ui->pteResponseHeaders->document()->setPlainText(headersText);

			UtilFRequest::SerializationFormatType serializationType = UtilFRequest::getSerializationFormatTypeForString(totalLoadedData);

			if(ui->actionFormat_Response_Body->isChecked()){
				ui->pteResponseBody->document()->setPlainText(UtilFRequest::getStringFormattedForSerializationType(totalLoadedData, serializationType));
				formatResponseBody(serializationType);
			}
			else{
				ui->pteResponseBody->document()->setPlainText(totalLoadedData);
			}

			// Check if there are more bytes to read
			// (they will not be displayed on the interface, but they can be saved to a file and a warning in the interface will be displayed)
			currentData = reply->read(maxBytesForBufferAndDisplay);
			if(currentData.size() > 0){
				totalLoadedData.append(currentData);
				ui->lbResponseBodyWarningIcon->show();
				ui->lbResponseBodyWarningMessage->show();
			}

			if(ui->cbDownloadResponseAsFile->isChecked())
			{

				QString filePath;
				QString fileName;

				if(ui->actionUse_Last_Download_Location->isChecked() && !this->lastResponseFileName.isEmpty())
				{
					fileName = this->lastResponseFileName;
					filePath = this->currentSettings.lastResponseFilePath + "/" + this->lastResponseFileName;
				}
				else
				{
					fileName = getDownloadFileName(reply);
					filePath = QFileDialog::getSaveFileName(this, tr("Save File"),
															this->currentSettings.lastResponseFilePath + "/" + fileName);
				}

				if(!filePath.isEmpty())
				{
					this->currentSettings.lastResponseFilePath = Util::FileSystem::normalizePath(QFileInfo(filePath).absoluteDir().path());
					this->lastResponseFileName = Util::FileSystem::cutNameWithoutBackSlash(Util::FileSystem::normalizePath(filePath));

					QFile file(filePath);

					if (file.open(QIODevice::WriteOnly)) {
						file.write(totalLoadedData);

						// Load remaining data using a buffer
						do{
							currentData = reply->read(maxBytesForBufferAndDisplay);
							file.write(currentData);
						}while(currentData.size() > 0);

						file.close();

						if(ui->actionOpen_file_after_download->isChecked()){
							if(!QDesktopServices::openUrl("file:///"+filePath)){
								QString errorMessage = "Could not open downloaded file: " + filePath;
								Util::Dialogs::showError(errorMessage);
								LOG_ERROR << errorMessage;
								Util::StatusBar::showError(ui->statusBar, errorMessage);
							}
						}

						Util::StatusBar::showSuccess(ui->statusBar, "File saved with success.");
					}
					else{ // use just one exit point so we don't need to duplicate the code to enable the send request button
						QString errorMessage = "Could not open file for writing: " + filePath;
						Util::Dialogs::showError(errorMessage);
						Util::StatusBar::showSuccess(ui->statusBar, errorMessage);
						LOG_ERROR << errorMessage;
					}
				}
			}
			else{
				QString successMessage = "Request performed with success.";
				
				if(this->currentProjectItem->authData != nullptr && ui->cbRequestOverrideMainUrl->isChecked()){
					successMessage += " Since this was an url overriden request, the authentication was not applied.";
				}
				
				Util::StatusBar::showSuccess(ui->statusBar, successMessage);
			}
		}
		else{
			Util::StatusBar::showSuccess(ui->statusBar, "Authenticated with success.");
		}
	}
    else if(reply->error() == QNetworkReply::OperationCanceledError){ /* ignore user aborted */ }
    else{
		
		// If we receive 401 and if we have an authentication and if we are allowed to retry to authenticate again, repeat the request
		if
		(
			requestReturnCode == "401" && this->currentProjectItem->authData != nullptr && 
			this->currentProjectItem->authData->retryLoginIfError401 &&
			this->currentProjectItem->authData->type == FRequestAuthentication::AuthenticationType::REQUEST_AUTHENTICATION && 
			this->currentProjectAuthenticationWasMade
		){
			isToRetryWithAuthentication = true;
		}
		
		if(!isToRetryWithAuthentication){
			
			QString requestType;
			bool overridenRequest = !this->authenticationIsRunning && this->currentProjectItem->authData != nullptr && ui->cbRequestOverrideMainUrl->isChecked();
			
			if(this->authenticationIsRunning){
				requestType = "Authentication";
			}
			else{
				requestType = "Request";
			}
			
			LOG_ERROR << requestReturnMessage;

			Util::Dialogs::showError("An error occurred while performing the " + requestType + ".\n" + requestReturnMessage);
			Util::StatusBar::showError(ui->statusBar, requestType + " was not performed with success." + 
			(overridenRequest ? " Since this was an url overriden request, the authentication was not applied." : ""));
		}
		
		isError = true;

    }

    this->lbRequestInfo.hide();
	this->pbRequestProgress.hide();
	this->tbAbortRequest.hide();
	ui->pbSendRequest->setEnabled(true);
	ui->gbRequest->setEnabled(true);
	ui->gbProject->setEnabled(true);
	this->currentReply.reset();
	reply->deleteLater();
	
	if(this->authenticationIsRunning){
		this->authenticationIsRunning = false;
		
		if(!isError){
			this->currentProjectAuthenticationWasMade = true;
		}
		
	}
	
	emit signalRequestFinishedAndProcessed();
	
	if(isToRetryWithAuthentication){
		this->currentProjectAuthenticationWasMade = false;
		on_pbSendRequest_clicked();
	}
	
}

void MainWindow::on_leRequestOverrideMainUrl_textChanged(const QString &)
{
    setProjectHasChanged();

    if(ui->cbRequestOverrideMainUrl->isChecked()){
        buildFullPath();
    }
}

void MainWindow::buildFullPath(){

    QString normalizedMainUrl;

    if(ui->cbRequestOverrideMainUrl->isChecked()){
        normalizedMainUrl = ui->leRequestOverrideMainUrl->text();
    }
    else{
        if(this->currentProjectItem != nullptr){
            normalizedMainUrl = this->currentProjectItem->projectMainUrl;
        }
    }

    ui->leFullPath->setText(getFullPathFromMainUrlAndPath(normalizedMainUrl, ui->lePath->text()));
}

QString MainWindow::getFullPathFromMainUrlAndPath(const QString & mainUrl, const QString & path){
    QString normalizedMainUrl = mainUrl;

    if(!normalizedMainUrl.endsWith('/') && !path.startsWith('/')){
        normalizedMainUrl += '/';
    }

    return normalizedMainUrl + path;
}

void MainWindow::on_lePath_textChanged(const QString &)
{
    setProjectHasChanged();

    buildFullPath();
}

QNetworkReply* MainWindow::processHttpRequest
(
        const UtilFRequest::RequestType requestType,
        const QString &fullPath,
        const QString &bodyType,
        const QString &requestBody,
        const QVector<UtilFRequest::HttpHeader>& requestHeaders
        )
{
    std::unique_ptr<HttpRequest> httpRequest = nullptr;

    switch(requestType){
    case UtilFRequest::RequestType::GET_OPTION:
    {
        httpRequest = std::make_unique<GetHttpRequest>(&this->networkAccessManager, fullPath, requestHeaders);
        break;
    }
    case UtilFRequest::RequestType::POST_OPTION:
    {
        httpRequest = std::make_unique<PostHttpRequest>(&this->networkAccessManager, ui->twRequestBodyKeyValue, fullPath , bodyType, requestBody, requestHeaders);
        break;
    }
    case UtilFRequest::RequestType::PUT_OPTION:
    {
        httpRequest = std::make_unique<PutHttpRequest>(&this->networkAccessManager, ui->twRequestBodyKeyValue, fullPath , bodyType, requestBody, requestHeaders);
        break;
    }
    case UtilFRequest::RequestType::DELETE_OPTION:
    {
        httpRequest = std::make_unique<DeleteHttpRequest>(&this->networkAccessManager, fullPath, requestHeaders);
        break;
    }
    case UtilFRequest::RequestType::PATCH_OPTION:
    {
        httpRequest = std::make_unique<PatchHttpRequest>(&this->networkAccessManager, ui->twRequestBodyKeyValue, fullPath , bodyType, requestBody, requestHeaders);
        break;
    }
    case UtilFRequest::RequestType::HEAD_OPTION:
    {
        httpRequest = std::make_unique<HeadHttpRequest>(&this->networkAccessManager, fullPath, requestHeaders);
        break;
    }
    case UtilFRequest::RequestType::TRACE_OPTION:
    {
        httpRequest = std::make_unique<TraceHttpRequest>(&this->networkAccessManager, ui->twRequestBodyKeyValue, fullPath , bodyType, requestBody, requestHeaders);
        break;
    }
    case UtilFRequest::RequestType::OPTIONS_OPTION:
    {
        httpRequest = std::make_unique<OptionsHttpRequest>(&this->networkAccessManager, ui->twRequestBodyKeyValue, fullPath , bodyType, requestBody, requestHeaders);
        break;
    }
    default:{
        QString errorMessage = "Request type unknown: '" + ui->cbRequestType->currentText() + "'. Application must exit.";
        Util::Dialogs::showError(errorMessage);
        LOG_FATAL << errorMessage;
        exit(1);
    }
    }

    return httpRequest->processRequest();
}

QString MainWindow::getDownloadFileName(const QNetworkReply * const reply)
{
    QString fileFilename;

    const QString contentDisposition = reply->rawHeader("Content-Disposition");

    const QString stringToFind = "filename=";

    int fileNameIndex = contentDisposition.indexOf(stringToFind) + stringToFind.size();

    // If we have the filename in content disposition...
    if(fileNameIndex != -1){
        fileFilename = contentDisposition.mid(fileNameIndex); // assume the name is the remaining string after "filename="
        fileFilename = fileFilename.replace("\"","").replace(";",""); // remove any " or ;
    }

    // use the url filename if content disposition is not available
    if(fileFilename.isEmpty())
    {
        QString path = reply->url().path();
        fileFilename = QFileInfo(path).fileName();

        if (fileFilename.isEmpty())
            fileFilename = "download";
    }

    return fileFilename;
}

void MainWindow::on_treeWidget_customContextMenuRequested(const QPoint &pos)
{
    QTreeWidget *myTree = ui->treeWidget;

    QList<int> selectedRows = QList<int>();

    for(const QModelIndex &rowItem : myTree->selectionModel()->selectedRows()){
        selectedRows << rowItem.row();
    }

    std::unique_ptr<QMenu> menu = std::make_unique<QMenu>();

    // Project actions only
    std::unique_ptr<QAction> openProjectLocation = nullptr;
    std::unique_ptr<QAction> projectProperties = nullptr;

    // Requests actions only
    std::unique_ptr<QAction> cloneRequest = nullptr;
    std::unique_ptr<QAction> moveRequestUp = nullptr;
    std::unique_ptr<QAction> moveRequestDown = nullptr;
    std::unique_ptr<QAction> deleteRequest = nullptr;

    // Common actions
    std::unique_ptr<QAction> addNewRequest = std::make_unique<QAction>("Add new request", myTree);
	std::unique_ptr<QAction> renameItem = nullptr;

    if(ui->treeWidget->currentItem() == this->currentProjectItem){ // Project
		renameItem = std::make_unique<QAction>("Rename project", myTree);
        openProjectLocation = std::make_unique<QAction>("Open project location", myTree);
        menu->addAction(openProjectLocation.get());
        menu->addSeparator();
        menu->addAction(addNewRequest.get());
		menu->addAction(renameItem.get());
        menu->addSeparator();
        projectProperties = std::make_unique<QAction>("Project properties", myTree);
        menu->addAction(projectProperties.get());
    }
    else{ // Requests
		renameItem = std::make_unique<QAction>("Rename request", myTree);
        menu->addAction(addNewRequest.get());
		menu->addAction(renameItem.get());
        cloneRequest =  std::make_unique<QAction>(QIcon(":/icons/clone_icon.png"), "Clone request", myTree);
        menu->addAction(cloneRequest.get());
        menu->addSeparator();
        moveRequestUp =  std::make_unique<QAction>("Move up", myTree);
        menu->addAction(moveRequestUp.get());
        moveRequestDown =  std::make_unique<QAction>("Move down", myTree);
        menu->addAction(moveRequestDown.get());
        menu->addSeparator();
        deleteRequest =  std::make_unique<QAction>(QIcon(":/icons/delete_icon.png"), "Delete request", myTree);
        menu->addAction(deleteRequest.get());

        if(this->currentProjectItem->childCount() == 1){
            moveRequestDown->setEnabled(false);
            moveRequestUp->setEnabled(false);
        }
        else if(ui->treeWidget->itemAbove(ui->treeWidget->currentItem()) == this->currentProjectItem){
            moveRequestUp->setEnabled(false);
        }
        else if(ui->treeWidget->itemBelow(ui->treeWidget->currentItem()) == nullptr){
            moveRequestDown->setEnabled(false);
        }
    }

	// Shortcuts info display for the users
	renameItem->setShortcut(Qt::Key_F2);
	renameItem->setShortcutVisibleInContextMenu(true);
	
	if(deleteRequest != nullptr){
		deleteRequest->setShortcut(Qt::Key_Delete);
		deleteRequest->setShortcutVisibleInContextMenu(true);
	}
	
    // Disable show in explorer if we don't have any project saved to disk
    if(openProjectLocation != nullptr && this->lastProjectFilePath.isEmpty()){
        openProjectLocation->setEnabled(false);
    }

    QAction* selectedOption = menu->exec(myTree->viewport()->mapToGlobal(pos));

    // if none selected just return
    if(selectedOption == nullptr){
        return;
    }

    // Index delta is the difference between the current index and the new one
    auto fMoveQTreeWidgetItem = [](QTreeWidgetItem *item, int indexDelta){
        // Based from here:
        // http://www.qtcentre.org/threads/56247-Moving-QTreeWidgetItem-Up-and-Down-in-a-QTreeWidget
        QTreeWidgetItem* parent = item->parent();
        int index = parent->indexOfChild(item);
        QTreeWidgetItem* child = parent->takeChild(index);
        parent->insertChild(index+indexDelta, child);
        item->treeWidget()->clearSelection();
        item->setSelected(true);
        item->treeWidget()->setCurrentItem(item);
    };

    if(selectedOption == addNewRequest.get()){

        this->ignoreAnyChangesToProject.SetCondition();
        this->unsavedChangesExist = true;

        FRequestTreeWidgetItem *newRequest = addRequestItem("New Request", getNewUuid(), this->currentProjectItem);
        ui->treeWidget->clearSelection();
        newRequest->setSelected(true);

        // Necessary in order to currentIndexChanged to work correctly (select it is not enough)
        ui->treeWidget->setCurrentItem(newRequest);

        // Reset options
        clearRequestAndResponse();

        ui->treeWidget->editItem(newRequest);

        if(this->currentSettings.defaultHeaders.useDefaultHeaders){
            addDefaultHeaders();
        }

        this->ignoreAnyChangesToProject.UnsetCondition();
    }
	else if(selectedOption == renameItem.get()){
		ui->treeWidget->editItem(ui->treeWidget->currentItem());
    }
    else if(selectedOption == cloneRequest.get()){
        this->unsavedChangesExist = true;

        const FRequestTreeWidgetRequestItem * const itemToClone = FRequestTreeWidgetRequestItem::fromQTreeWidgetItem(ui->treeWidget->currentItem());

        FRequestTreeWidgetRequestItem *newRequest = addRequestItem(itemToClone->text(0), getNewUuid(), this->currentProjectItem);
        ui->treeWidget->clearSelection();
        newRequest->setSelected(true);

        // Necessary in order to currentIndexChanged to work correctly (select it is not enough)
        ui->treeWidget->setCurrentItem(newRequest);

        this->currentItem = newRequest;
    }
    else if(openProjectLocation != nullptr && selectedOption == openProjectLocation.get()){
        QDesktopServices::openUrl(QUrl("file:///" + QFileInfo(this->lastProjectFilePath).absoluteDir().absolutePath()));
    }
    else if(selectedOption == moveRequestUp.get()){
        fMoveQTreeWidgetItem(ui->treeWidget->currentItem(), -1);
        setProjectHasChanged();
    }
    else if(selectedOption == moveRequestDown.get()){
        fMoveQTreeWidgetItem(ui->treeWidget->currentItem(), 1);
        setProjectHasChanged();
    }
    else if(selectedOption == deleteRequest.get()){
        removeRequest(FRequestTreeWidgetRequestItem::fromQTreeWidgetItem(ui->treeWidget->currentItem()));
    }
    else if(selectedOption == projectProperties.get()){
		openProjectProperties();
    }
}

// This signal is emitted when the current item changes.
// The current item is specified by current, and this replaces the previous current item.
void MainWindow::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    FRequestTreeWidgetItem* currentFRequestItem = nullptr;
    FRequestTreeWidgetItem* previousFRequestItem = nullptr;

    if(current != nullptr){
        currentFRequestItem = FRequestTreeWidgetItem::fromQTreeWidgetItem(current);

        // Add the icon if necessary
        if(ui->actionShow_Request_Types_Icons->isChecked() && !currentFRequestItem->isProjectItem && currentFRequestItem->hasEmptyIcon()){
            setIconForRequest(static_cast<FRequestTreeWidgetRequestItem*>(currentFRequestItem));
        }
    }

    if(previous != nullptr){
        previousFRequestItem = FRequestTreeWidgetItem::fromQTreeWidgetItem(previous);
    }

    // Save previous item
    if(previousFRequestItem != nullptr && !previousFRequestItem->isProjectItem)
    {
        updateTreeWidgetItemContent(static_cast<FRequestTreeWidgetRequestItem*>(previousFRequestItem));
    }

    // Update window for new item
    if(currentFRequestItem != nullptr){

        if(!currentFRequestItem->isProjectItem){

            this->currentItem = static_cast<FRequestTreeWidgetRequestItem*>(currentFRequestItem);

            if(!this->ignoreAnyChangesToProject.ConditionIsSet()){
                reloadRequest(this->currentItem);
            }

            // Hide project requests number if project item hasn't selected
            if(!this->lbProjectInfo.text().isEmpty()){
                this->lbProjectInfo.setText(QString());
            }
        }
        else{
            // If is the project display the number of requests that this has in status bar
            const int projectRequestsNumber = this->currentProjectItem->childCount();

            // Singular ? Plural ?
            const QString requestText = projectRequestsNumber == 1 ? " request" : " requests";

            this->lbProjectInfo.setText(this->currentProjectItem->projectName + " has a total of " + QString::number(projectRequestsNumber) + requestText);
        }

        updateWindowTitle();
    }

}

// This signal is emitted when the contents of the column in the specified item changes.
void MainWindow::on_treeWidget_itemChanged(QTreeWidgetItem *item, int)
{
    FRequestTreeWidgetItem *newItem = FRequestTreeWidgetItem::fromQTreeWidgetItem(item);

    setProjectHasChanged();

    bool updateInterface = false;

    // Only necessary to create if the item exists (project requests are only created in on_treeWidget_currentItemChanged)

    // If name had changed update the project requests

    if(!newItem->isProjectItem){ // Request

        FRequestTreeWidgetRequestItem * const newItemRequest = static_cast<FRequestTreeWidgetRequestItem*>(newItem);

        if(item->text(0) != newItemRequest->itemContent.name){
            updateTreeWidgetItemContent(newItemRequest); // update request content
            updateInterface = true;
        }
    }
    else{ // Project

        FRequestTreeWidgetProjectItem * const newItemProject = static_cast<FRequestTreeWidgetProjectItem*>(newItem);

        if(item->text(0) != newItemProject->projectName){
            newItemProject->projectName = item->text(0); // update project name
            updateInterface = true;
        }

    }

    if(updateInterface){
        // Set item tooltip (same as text)
        item->setToolTip(0, item->text(0));

        updateWindowTitle();
    }
}

void MainWindow::updateWindowTitle(){

    // using QStringBuilder concatenation method:
    // https://wiki.qt.io/Using_QString_Effectively
    QString fRequestTitle;

    if(this->lastProjectFilePath.isEmpty()){
        fRequestTitle = fRequestTitle % "Untitled";
    }
    else{
        fRequestTitle = fRequestTitle % Util::FileSystem::cutNameWithoutBackSlash(this->lastProjectFilePath);
    }

    fRequestTitle = fRequestTitle % " - " % this->currentProjectItem->projectName;

    if(this->currentItem != nullptr){
        fRequestTitle = fRequestTitle % "/" % this->currentItem->text(0);
    }

    fRequestTitle = fRequestTitle % " - " % GlobalVars::AppName % " " % GlobalVars::AppVersion;

    if(this->unsavedChangesExist){
        fRequestTitle = fRequestTitle % "*";
    }

    setWindowTitle(fRequestTitle);
}

FRequestTreeWidgetProjectItem* MainWindow::addProjectItem(const QString &projectName, const QString &projectUuid){
    FRequestTreeWidgetProjectItem *projectFolder = new FRequestTreeWidgetProjectItem(QStringList() << projectName, projectUuid);
    projectFolder->setIcon(0, QIcon(":/icons/projects_folder_icon.png"));
    projectFolder->setToolTip(0, projectFolder->text(0));
    projectFolder->setFlags(projectFolder->flags() | Qt::ItemIsEditable);
    ui->treeWidget->addTopLevelItem(projectFolder);
    projectFolder->setExpanded(true);

    return projectFolder;
}

FRequestTreeWidgetRequestItem* MainWindow::addRequestItem(const QString &requestName, const QString &projectUuid, FRequestTreeWidgetProjectItem * const currentProject){
    FRequestTreeWidgetRequestItem *newRequest = new FRequestTreeWidgetRequestItem(QStringList() << requestName, projectUuid);
    newRequest->setToolTip(0, newRequest->text(0));
    newRequest->setFlags((newRequest->flags() | Qt::ItemIsEditable) ^ Qt::ItemIsDropEnabled); // don't allow drop inside another items
    currentProject->addChild(newRequest);

    return newRequest;
}

void MainWindow::setIconForRequest(FRequestTreeWidgetRequestItem * const item){

    if(this->generatedIconCache.contains(item->itemContent.requestType)){
        item->setIcon(0, this->generatedIconCache.value(item->itemContent.requestType));
    }
    else{ // generate the necessary icon and add it to our cache
        QPixmap myIcon(32,16);
        myIcon.fill(Qt::transparent);

        QPainter painter( &myIcon );
        painter.setFont(item->font(0));

        QString textToDraw;
        std::unique_ptr<QPen> colorToDraw;

        switch(item->itemContent.requestType){
        case UtilFRequest::RequestType::GET_OPTION:{
            textToDraw = "GET";
            colorToDraw = std::make_unique<QPen>(Qt::blue);
            break;
        }
        case UtilFRequest::RequestType::POST_OPTION:{
            textToDraw = "POST";
            colorToDraw = std::make_unique<QPen>(Qt::darkGreen);
            break;
        }
        case UtilFRequest::RequestType::PUT_OPTION:{
            textToDraw = "PUT";
            colorToDraw = std::make_unique<QPen>(0xFFAD00);
            break;
        }
        case UtilFRequest::RequestType::DELETE_OPTION:{
            textToDraw = "DEL";
            colorToDraw = std::make_unique<QPen>(Qt::red);
            break;
        }
        case UtilFRequest::RequestType::PATCH_OPTION:{
            textToDraw = "PTCH";
            colorToDraw = std::make_unique<QPen>(Qt::magenta);
            break;
        }
        case UtilFRequest::RequestType::HEAD_OPTION:{
            textToDraw = "HEAD";
            colorToDraw = std::make_unique<QPen>(0x4EC995);
            break;
        }
        case UtilFRequest::RequestType::TRACE_OPTION:{
            textToDraw = "TRCE";
            colorToDraw = std::make_unique<QPen>(Qt::darkYellow);
            break;
        }
        case UtilFRequest::RequestType::OPTIONS_OPTION:{
            textToDraw = "OPT";
            colorToDraw = std::make_unique<QPen>(Qt::darkGray);
            break;
        }
        default:{
            QString errorMessage = "Couldn't set icon for request " + item->text(0) +
                    " unknown request type: " + QString::number(static_cast<int>(item->itemContent.requestType));
            LOG_ERROR << errorMessage;
            Util::Dialogs::showError(errorMessage);
            return;
        }
        }

        painter.setPen(*colorToDraw);
        painter.drawText( QRect(0, 0, myIcon.width(), myIcon.height()), Qt::AlignCenter, textToDraw );

        item->setIcon(0, *this->generatedIconCache.insert(item->itemContent.requestType, QIcon(myIcon)));
    }

}

void MainWindow::setNewProject(){

    if(this->unsavedChangesExist){
        QMessageBox::StandardButton result = askToSaveCurrentProject();
        if(result == QMessageBox::StandardButton::Cancel){
            return;
        }
    }

    this->ignoreAnyChangesToProject.SetCondition();

    clearEverything();

    FRequestTreeWidgetProjectItem *projectFolder = addProjectItem("New Project", getNewUuid());
    FRequestTreeWidgetRequestItem *newRequest = addRequestItem("New Request", getNewUuid(), projectFolder);

    newRequest->setSelected(true);

    this->currentProjectItem = projectFolder;
    this->currentItem = newRequest;

    this->currentProjectItem->projectName = projectFolder->text(0);

    // Necessary in order to currentIndexChanged to work correctly (select it is not enough)
    ui->treeWidget->setCurrentItem(newRequest);

    this->lastProjectFilePath = QString();

    if(this->currentSettings.defaultHeaders.useDefaultHeaders){
        addDefaultHeaders();
    }

    this->unsavedChangesExist = false;
    updateWindowTitle(); // it doesn't get called automatically here

    this->ignoreAnyChangesToProject.UnsetCondition();
	
	
}

QVector<UtilFRequest::HttpHeader> MainWindow::getRequestHeaders(){

    QVector<UtilFRequest::HttpHeader> requestHeaders;

    for(int i = 0; i < ui->twRequestHeadersKeyValue->rowCount(); i++){

        UtilFRequest::HttpHeader currentHeader;

        currentHeader.name = ui->twRequestHeadersKeyValue->item(i, 0)->text();
        currentHeader.value = ui->twRequestHeadersKeyValue->item(i, 1)->text();

        requestHeaders.append(currentHeader);
    }

    return requestHeaders;
}

QVector<UtilFRequest::HttpFormKeyValueType> MainWindow::getRequestForm(){

    QVector<UtilFRequest::HttpFormKeyValueType> requestForm;

    for(int i = 0; i < ui->twRequestBodyKeyValue->rowCount(); i++){

        // TODO use enum for index
        UtilFRequest::HttpFormKeyValueType currentFormKeyValue(
                    ui->twRequestBodyKeyValue->item(i, 0)->text(),
                    ui->twRequestBodyKeyValue->item(i, 1)->text(),
                    UtilFRequest::getFormKeyTypeByString(ui->twRequestBodyKeyValue->item(i, 2)->text())
                    );

        requestForm.append(currentFormKeyValue);
    }

    return requestForm;
}


void MainWindow::updateTreeWidgetItemContent(FRequestTreeWidgetRequestItem * const requestItem){
    requestItem->itemContent.name = requestItem->text(0);
    requestItem->itemContent.path = ui->lePath->text();

    requestItem->itemContent.bOverridesMainUrl = ui->cbRequestOverrideMainUrl->isChecked();
    requestItem->itemContent.overrideMainUrl = ui->leRequestOverrideMainUrl->text();

    requestItem->itemContent.headers = getRequestHeaders();

    requestItem->itemContent.order = requestItem->parent()->indexOfChild(requestItem);

    requestItem->itemContent.bodyType = UtilFRequest::getBodyTypeByString(ui->cbBodyType->currentText());

    switch(requestItem->itemContent.bodyType){
    case UtilFRequest::BodyType::RAW:
    {
        requestItem->itemContent.body = ui->pteRequestBody->toPlainText();
        break;
    }
        break;
    case UtilFRequest::BodyType::FORM_DATA:
    {
        requestItem->itemContent.bodyForm = getRequestForm();
        break;
    }
        break;
    case UtilFRequest::BodyType::X_FORM_WWW_URLENCODED:
    {
        requestItem->itemContent.bodyForm = getRequestForm();
        break;
    }
    default:
    {
        QString errorMessage = "Invalid body type " + QString::number(static_cast<int>(requestItem->itemContent.bodyType)) + "'. Program can't proceed.";
        Util::Dialogs::showError(errorMessage);
        LOG_FATAL << errorMessage;
        exit(1);
    }
    }

    requestItem->itemContent.requestType = UtilFRequest::getRequestTypeByString(ui->cbRequestType->currentText());

    requestItem->itemContent.bDownloadResponseAsFile = ui->cbDownloadResponseAsFile->isChecked();
}

void MainWindow::saveProjectState(const QString &filePath)
{

    // Make sure the current request is updated in memory
    if(this->currentItem != nullptr){
        updateTreeWidgetItemContent(this->currentItem);
    }

    try{
        ProjectFileFRequest::saveProjectDataToFile(filePath, fetchCurrentProjectData(), this->uuidsToCleanUp);

        this->currentSettings.lastProjectPath = QFileInfo(filePath).absoluteDir().path();

        this->lastProjectFilePath = filePath;
        this->uuidsToCleanUp.clear();
        this->unsavedChangesExist = false;
        addNewRecentProject(filePath);

        updateWindowTitle();

        Util::StatusBar::showSuccess(ui->statusBar, "Project saved with success!");
    }
    catch(const std::exception& e){
        QString errorMessage = QString("Couldn't save project file. Save aborted.\n") + e.what();
        LOG_ERROR << errorMessage;
        Util::Dialogs::showError(errorMessage);
        Util::StatusBar::showError(ui->statusBar, "Couldn't save project file.");
    }
}

void MainWindow::loadProjectState(const QString &filePath)
{

    if(this->unsavedChangesExist){
        QMessageBox::StandardButton result = askToSaveCurrentProject();
        if(result == QMessageBox::StandardButton::Cancel){
            return;
        }
    }

    // Prepare for project loading
    this->ignoreAnyChangesToProject.SetCondition();

    try{
        std::unique_ptr<ProjectFileFRequest::ProjectData> projectData =
                std::make_unique<ProjectFileFRequest::ProjectData>(ProjectFileFRequest::readProjectDataFromFile(filePath));

        clearEverything();

        this->currentProjectItem = addProjectItem(projectData->projectName, projectData->projectUuid);
        this->currentProjectItem->projectName = projectData->projectName;
        this->currentProjectItem->projectMainUrl = projectData->mainUrl;
        this->currentProjectItem->authData = projectData->authData;

        // Order them by the correct order
        std::sort(
                    projectData->projectRequests.begin(),
                    projectData->projectRequests.end(),
                    [](const UtilFRequest::RequestInfo &first, const UtilFRequest::RequestInfo &second){ return first.order < second.order; }
        );

        // Now the items are in the correct order, we can now load them in the interface
        for(int i=0; i<projectData->projectRequests.size(); i++){

            UtilFRequest::RequestInfo &currentRequestInfo = projectData->projectRequests[i];

            // fix order
            currentRequestInfo.order = i;

            FRequestTreeWidgetRequestItem *currRequest = addRequestItem(currentRequestInfo.name, currentRequestInfo.uuid, this->currentProjectItem);
            currRequest->itemContent = currentRequestInfo;

            if(ui->actionShow_Request_Types_Icons->isChecked()){
                setIconForRequest(currRequest);
            }

            // Load first request
            if(i==0){
                reloadRequest(currRequest);
                ui->treeWidget->setCurrentItem(currRequest);
                this->currentItem = currRequest;
            }

        }

        // All items loaded, finally check if we don't have an authentication yet.
        // If we don't, check if it is present in config file and load it from there

        if(this->currentProjectItem->authData == nullptr &&
                this->currentSettings.mapOfConfigAuths_UuidToConfigAuth.contains(this->currentProjectItem->getUuid())){
            // Project has auth data in config file, load it

            this->currentProjectItem->authData = this->currentSettings.mapOfConfigAuths_UuidToConfigAuth.value(this->currentProjectItem->getUuid()).authData;
        }

        this->currentSettings.lastProjectPath = QFileInfo(filePath).absoluteDir().path();

        this->lastProjectFilePath = filePath;
        this->unsavedChangesExist = false;

        this->currentProjectAuthenticationWasMade = false;

        addNewRecentProject(filePath);

        updateWindowTitle();

        Util::StatusBar::showSuccess(ui->statusBar, "Project loaded successfully.");
    }
    catch(const std::exception& e){
        QString errorMessage = "Couldn't load the FRequest project. Error: " + QString(e.what());
        LOG_ERROR << errorMessage;
        Util::Dialogs::showError(errorMessage);
        Util::StatusBar::showError(ui->statusBar, "Couldn't load project.");
    }

    this->ignoreAnyChangesToProject.UnsetCondition();
}

ProjectFileFRequest::ProjectData MainWindow::fetchCurrentProjectData(){
    ProjectFileFRequest::ProjectData currentProjectData;

    currentProjectData.projectName = this->currentProjectItem->projectName;
    currentProjectData.mainUrl = this->currentProjectItem->projectMainUrl;
    currentProjectData.projectUuid = this->currentProjectItem->getUuid();
    currentProjectData.authData = this->currentProjectItem->authData;

    // Save by the current tree order
    for(int i=0; i<this->currentProjectItem->childCount(); i++){

        UtilFRequest::RequestInfo &currentRequest = FRequestTreeWidgetRequestItem::fromQTreeWidgetItem(this->currentProjectItem->child(i))->itemContent;

        currentRequest.order = i;

        currentProjectData.projectRequests.append(currentRequest);
    }

    return currentProjectData;
}

void MainWindow::on_actionNew_Project_triggered()
{
    setNewProject();
}

void MainWindow::on_actionSave_Project_triggered()
{
    if(this->lastProjectFilePath.isEmpty()){
        on_actionSave_Project_As_triggered();
        return;
    }

    saveProjectState(this->lastProjectFilePath);
}

void MainWindow::on_actionSave_Project_As_triggered()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                    this->currentSettings.lastProjectPath,
                                                    tr("FRequest project files (*.frp)"));

    if(!filePath.isEmpty()){
        saveProjectState(filePath);
    }
}

void MainWindow::on_actionLoad_Project_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Load File"),
                                                    this->currentSettings.lastProjectPath,
                                                    tr("FRequest project files (*.frp)"));
    if(!filePath.isEmpty()){
        loadProjectState(filePath);
    }
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_actionAbout_triggered()
{
    //Show about dialog
    (new About(this))->show(); //it destroys itself when finished.
}

void MainWindow::on_actionProject1_triggered()
{
    loadProjectState(this->ui->actionProject1->text());
}

void MainWindow::on_actionProject2_triggered()
{
    loadProjectState(this->ui->actionProject2->text());
}

void MainWindow::on_actionProject3_triggered()
{
    loadProjectState(this->ui->actionProject3->text());
}

void MainWindow::on_actionProject4_triggered()
{
    loadProjectState(this->ui->actionProject4->text());
}

void MainWindow::on_actionProject5_triggered()
{
    loadProjectState(this->ui->actionProject5->text());
}

void MainWindow::on_actionProject6_triggered()
{
    loadProjectState(this->ui->actionProject6->text());
}

void MainWindow::on_actionFormat_Response_Body_triggered()
{
    formatResponseBody(getResponseCurrentSerializationFormatType());
}

void MainWindow::on_actionFormat_Request_body_triggered()
{
    this->ignoreAnyChangesToProject.SetCondition();
    formatRequestBody(getRequestCurrentSerializationFormatType());
    this->ignoreAnyChangesToProject.UnsetCondition();
}

void MainWindow::reloadRequest(FRequestTreeWidgetRequestItem* const item){

    this->ignoreAnyChangesToProject.SetCondition();

    UtilFRequest::RequestInfo info = item->itemContent;

    // Clear old contents
    clearRequestAndResponse();

    ui->lePath->setText(info.path);
    ui->cbRequestOverrideMainUrl->setChecked(info.bOverridesMainUrl);
    ui->leRequestOverrideMainUrl->setText(info.overrideMainUrl);

    setRequestType(info.requestType);

    // Aux lambda to avoid duplicated code for FORM_DATA / X_FORM_WWW_URLENCODED
    auto fFillRequestBodyKeyValueTable = [](const QVector<UtilFRequest::HttpFormKeyValueType> &bodyForm, QTableWidget * const table){
        for(const UtilFRequest::HttpFormKeyValueType &currFormKeyValue : bodyForm){
            UtilFRequest::addRequestFormBodyRow(table, currFormKeyValue.key, currFormKeyValue.value, currFormKeyValue.type);
        }
    };

    ui->cbBodyType->setCurrentText(UtilFRequest::getBodyTypeString(info.bodyType));

    switch(info.bodyType){
    case UtilFRequest::BodyType::RAW:
    {
        ui->pteRequestBody->setPlainText(info.body);
        break;
    }
        break;
    case UtilFRequest::BodyType::FORM_DATA:
    {
        fFillRequestBodyKeyValueTable(info.bodyForm, ui->twRequestBodyKeyValue);
        break;
    }
        break;
    case UtilFRequest::BodyType::X_FORM_WWW_URLENCODED:
    {
        fFillRequestBodyKeyValueTable(info.bodyForm, ui->twRequestBodyKeyValue);
        break;
    }
    default:
    {
        QString errorMessage = "Invalid body type " + QString::number(static_cast<int>(info.bodyType)) + "'. Program can't proceed.";
        Util::Dialogs::showError(errorMessage);
        LOG_FATAL << errorMessage;
        exit(1);
    }
    }

    ui->pteRequestBody->setPlainText(info.body);

    formatRequestBody(getRequestCurrentSerializationFormatType());

    for(const UtilFRequest::HttpHeader &currentHeader : info.headers){
        Util::TableWidget::addRow(ui->twRequestHeadersKeyValue, QStringList() << currentHeader.name << currentHeader.value);
    }

    ui->cbDownloadResponseAsFile->setChecked(info.bDownloadResponseAsFile);

    clearOlderResponse();

    this->ignoreAnyChangesToProject.UnsetCondition();
}

void MainWindow::clearOlderResponse(){
    this->lastReplyStatusError = 0;
    ui->lbResponseBodyWarningIcon->hide();
    ui->lbResponseBodyWarningMessage->hide();
    ui->pteResponseBody->clear();
    ui->pteResponseHeaders->clear();
    ui->lbStatus->clear();
    ui->lbDescription->clear();
    ui->lbTimeElapsed->clear();
}

void MainWindow::clearRequestAndResponse(){
    ui->leRequestOverrideMainUrl->clear();
    ui->cbRequestOverrideMainUrl->setChecked(false);
    ui->lePath->clear();
    ui->leFullPath->clear();
    ui->cbRequestType->setCurrentText("GET");
    ui->cbBodyType->setCurrentIndex(0);
    ui->cbResponseChooseHeader->setCurrentIndex(0);
    ui->pteRequestBody->clear();
    Util::TableWidget::clearContentsNoPrompt(ui->twRequestBodyKeyValue);
    Util::TableWidget::clearContentsNoPrompt(ui->twRequestHeadersKeyValue);
    ui->cbDownloadResponseAsFile->setChecked(false);

    clearOlderResponse();
}

void MainWindow::clearEverything(){
    // order is important
    // (we should clear the pointers first, because clearing tree widgets
    // will then call update title that will try to access pointer to unexisting objects)
    this->uuidsInUse.clear();
    this->uuidsToCleanUp.clear();
    this->currentItem = nullptr;
    this->currentProjectItem = nullptr;
    ui->treeWidget->clear();
	ui->leRequestsFilter->clear();
    clearRequestAndResponse();
}

void MainWindow::on_cbResponseChooseHeader_currentIndexChanged(const QString &arg1)
{

    if(arg1 == "Content-type: application/json"){
        Util::TableWidget::addRow(ui->twRequestHeadersKeyValue, QStringList() << "Content-type" << "application/json");
    }
    else if(arg1 == "Content-type: application/xml"){
        Util::TableWidget::addRow(ui->twRequestHeadersKeyValue, QStringList() << "Content-type" << "application/xml");
    }
    else if(arg1 == "Content-type: multipart/form-data"){
        Util::TableWidget::addRow(ui->twRequestHeadersKeyValue, QStringList() << "Content-type" << "multipart/form-data");
    }
    else if(arg1 == "Content-type: application/x-www-form-urlencoded"){
        Util::TableWidget::addRow(ui->twRequestHeadersKeyValue, QStringList() << "Content-type" << "application/x-www-form-urlencoded");
    }

    ui->cbResponseChooseHeader->setCurrentIndex(0);
}

void MainWindow::closeEvent(QCloseEvent *event){

    if(this->unsavedChangesExist){
        QMessageBox::StandardButton result = askToSaveCurrentProject();
        if(result == QMessageBox::StandardButton::Cancel){
            event->ignore();
            return;
        }
    }

    if(this->currentSettings.windowsGeometry.saveWindowsGeometryWhenExiting){
        this->currentSettings.windowsGeometry.mainWindow_MainWindowGeometry = this->saveGeometry();
        this->currentSettings.windowsGeometry.mainWindow_RequestsSplitterState = ui->splitter->saveState();
        this->currentSettings.windowsGeometry.mainWindow_RequestResponseSplitterState = ui->splitter_2->saveState();
    }

    // Save unsaved settings
    saveCurrentSettings();

    // Exit application (this will also close all other windows which don't have parent)
    QApplication::quit();
}

void MainWindow::loadRecentProjects(){
    for(const QString &currentRecentPath : this->currentSettings.recentProjectsPaths){
        recentProjectsList.append(currentRecentPath);
    }

    reloadRecentProjectsMenu();

}

void MainWindow::addNewRecentProject(const QString &filePath){

    // If the new project is equal to the last one simply ignore
    if(this->currentSettings.recentProjectsPaths.size() > 0 && filePath == this->currentSettings.recentProjectsPaths[0]){
        return;
    }

    // If the item already exists in our list remove it, so it can go to the top again
    for(auto it = this->recentProjectsList.begin(); it != this->recentProjectsList.end();){
        if(*it == filePath){
            it = this->recentProjectsList.erase(it);
        }
        else{
            it++;
        }
    }

    // if we gonna overflow our list, remove the older item to reserve space to the new one
    if(this->recentProjectsList.size()==GlobalVars::AppRecentProjectsMaxSize){
        this->recentProjectsList.removeLast();
    }

    this->currentSettings.lastProjectPath = QFileInfo(filePath).absoluteDir().path();

    // add new recent file
    this->recentProjectsList.prepend(filePath);

    reloadRecentProjectsMenu();

    saveRecentProjects();
}

void MainWindow::reloadRecentProjectsMenu(){

    ui->menuRecent_Projects->setEnabled(false);
    ui->actionProject1->setVisible(false);
    ui->actionProject2->setVisible(false);
    ui->actionProject3->setVisible(false);
    ui->actionProject4->setVisible(false);
    ui->actionProject5->setVisible(false);
    ui->actionProject6->setVisible(false);

    {
        QList<QString>::const_iterator it;
        int i;
        for(it = recentProjectsList.cbegin(), i=0; it != recentProjectsList.cend(); it++, i++){

            QAction* currAction = nullptr;

            switch (i){
            case 0:
                currAction = ui->actionProject1;
                break;
            case 1:
                currAction = ui->actionProject2;
                break;
            case 2:
                currAction = ui->actionProject3;
                break;
            case 3:
                currAction = ui->actionProject4;
                break;
            case 4:
                currAction = ui->actionProject5;
                break;
            case 5:
                currAction = ui->actionProject6;
                break;
            }

            if(currAction){
                ui->menuRecent_Projects->setEnabled(true);
                currAction->setText(*it);
                currAction->setVisible(true);
            }
        }
    }

}

void MainWindow::saveRecentProjects(){
    this->currentSettings.recentProjectsPaths = recentProjectsList.toVector();
}

void MainWindow::on_tbRequestHeadersKeyValueAdd_clicked()
{
    Util::TableWidget::addRow(ui->twRequestHeadersKeyValue, QStringList() << "" << "");
}

void MainWindow::on_tbRequestHeadersKeyValueRemove_clicked()
{
    int size = Util::TableWidget::getSelectedRows(ui->twRequestHeadersKeyValue).size();

    if(size==0){
        Util::Dialogs::showInfo("Select a row first!");
        return;
    }

    if(Util::Dialogs::showQuestion(this, "Are you sure you want to remove all selected rows?")){
        for(int i=0; i < size; i++){
            ui->twRequestHeadersKeyValue->removeRow(Util::TableWidget::getSelectedRows(ui->twRequestHeadersKeyValue).at(size-i-1).row());
        }

        Util::StatusBar::showSuccess(ui->statusBar, "Key-Value rows deleted");
    }
}

void MainWindow::on_twRequestHeadersKeyValue_cellChanged(int row, int column)
{
    setProjectHasChanged();

    if(!ui->twRequestHeadersKeyValue->item(row, column)->text().trimmed().isEmpty()){
        ui->twRequestHeadersKeyValue->resizeColumnsToContents();
    }
}

void MainWindow::on_tbRequestBodyKeyValueAdd_clicked()
{
    UtilFRequest::addRequestFormBodyRow(ui->twRequestBodyKeyValue, "", "", UtilFRequest::FormKeyValueType::TEXT);
}

void MainWindow::on_tbRequestBodyFile_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Choose a file to upload"));

    // Only add a row if a file was selected
    if(!filePath.isEmpty()){
        UtilFRequest::addRequestFormBodyRow(ui->twRequestBodyKeyValue, "", filePath, UtilFRequest::FormKeyValueType::FILE);
    }
}

void MainWindow::on_tbRequestBodyKeyValueRemove_clicked()
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

        Util::StatusBar::showSuccess(ui->statusBar, "Key-Value rows deleted");
    }
}

void MainWindow::on_twRequestBodyKeyValue_cellChanged(int row, int column)
{
    setProjectHasChanged();

    // TODO use column enums
    if(column != 1 && !ui->twRequestBodyKeyValue->item(row, column)->text().trimmed().isEmpty()){
        ui->twRequestBodyKeyValue->resizeColumnToContents(column);
    }
}

void MainWindow::on_cbBodyType_currentIndexChanged(const QString &arg1)
{
    auto fSetBodyKeyValuesTableVisibility = [=](const bool &isVisible){
        ui->twRequestBodyKeyValue->setVisible(isVisible);
        ui->tbRequestBodyKeyValueAdd->setVisible(isVisible);
        ui->tbRequestBodyFile->setVisible(isVisible);
        ui->tbRequestBodyKeyValueRemove->setVisible(isVisible);
    };

    // Files are only allowed in form-data, so we need to remove them if the user is changing to something else
    // Before removing them ask the user if that is ok
    if(!this->ignoreAnyChangesToProject.ConditionIsSet() && arg1 != "form-data" && formKeyValueInBodyHasFiles()){

        // revert to form data and don't do more anything if user doesn't approve files rows deletion
        if(Util::Dialogs::showQuestionWithCancel(this, "You have files rows in the form, changing to " + arg1 + " will REMOVE these files rows. Proceed?") != QMessageBox::Yes){
            this->ignoreAnyChangesToProject.SetCondition();
            ui->cbBodyType->setCurrentText("form-data");
            this->ignoreAnyChangesToProject.UnsetCondition();
            return;
        }
        else{
            removeAllFilesRowsFromFormKeyValueInBody(); // if approved remove the files rows
        }
    }

    ui->pteRequestBody->hide();
    fSetBodyKeyValuesTableVisibility(false);

    if(arg1 == "raw"){
        ui->pteRequestBody->show();
    }
    else if(arg1 == "form-data"){
        fSetBodyKeyValuesTableVisibility(true);
    }
    else if(arg1 == "x-form-www-urlencoded"){
        fSetBodyKeyValuesTableVisibility(true);
        ui->tbRequestBodyFile->setVisible(false);
    }

    if(!this->ignoreAnyChangesToProject.ConditionIsSet()){
        if(ui->twRequestHeadersKeyValue->rowCount() > 0 && (QMessageBox::Yes == Util::Dialogs::showQuestionWithCancel(this, "You have changed the request body type. Delete previous headers?"))){
            Util::TableWidget::clearContentsNoPrompt(ui->twRequestHeadersKeyValue);
        }

        if(this->currentSettings.defaultHeaders.useDefaultHeaders){
            addDefaultHeaders();
        }
    }
}

void MainWindow::on_cbRequestType_currentIndexChanged(const QString &)
{
    // Indicates if body can be set or not depending on the given option
    bool toogle = UtilFRequest::requestTypeMayHaveBody(UtilFRequest::getRequestTypeByString(ui->cbRequestType->currentText()));

    setProjectHasChanged();

    ui->cbBodyType->setEnabled(toogle);
    ui->twRequestBodyKeyValue->setEnabled(toogle);
    ui->pteRequestBody->setEnabled(toogle);
    ui->tbRequestBodyKeyValueAdd->setEnabled(toogle);
    ui->tbRequestBodyFile->setEnabled(toogle);
    ui->tbRequestBodyKeyValueRemove->setEnabled(toogle);

    // Save previous item (to update icon)
    if(this->currentItem != nullptr && !this->currentItem->isProjectItem)
    {
        updateTreeWidgetItemContent(this->currentItem);

        // Update icon
        if(ui->actionShow_Request_Types_Icons->isChecked()){
            setIconForRequest(this->currentItem);
        }
    }

    if(!this->ignoreAnyChangesToProject.ConditionIsSet()){
        if(ui->twRequestHeadersKeyValue->rowCount() > 0 && (QMessageBox::Yes == Util::Dialogs::showQuestionWithCancel(this, "You have changed the request type. Delete previous headers?"))){
            Util::TableWidget::clearContentsNoPrompt(ui->twRequestHeadersKeyValue);
        }

        if(this->currentSettings.defaultHeaders.useDefaultHeaders){
            addDefaultHeaders();
        }
    }
}

void MainWindow::on_cbRequestOverrideMainUrl_toggled(bool checked)
{
    setProjectHasChanged();

    ui->leRequestOverrideMainUrl->setEnabled(checked);

    buildFullPath();
}

void MainWindow::on_actionShow_Request_Types_Icons_triggered(bool checked)
{
    this->ignoreAnyChangesToProject.SetCondition();
    setAllRequestIcons(checked);
    this->currentSettings.showRequestTypesIcons = checked;
    this->ignoreAnyChangesToProject.UnsetCondition();
}

void MainWindow::setAllRequestIcons(bool showIcon){

    if(showIcon){
        // Set request icons
        for(int i=0; i<this->currentProjectItem->childCount(); i++){
            setIconForRequest(FRequestTreeWidgetRequestItem::fromQTreeWidgetItem(this->currentProjectItem->child(i)));
        }
    }
    else{
        // clear request icons
        for(int i=0; i<this->currentProjectItem->childCount(); i++){
            this->currentProjectItem->child(i)->setIcon(0, QIcon());
        }
    }

}

void MainWindow::on_pteRequestBody_textChanged()
{
    setProjectHasChanged();
}

void MainWindow::on_cbDownloadResponseAsFile_toggled(bool)
{
    setProjectHasChanged();
}

void MainWindow::tbAbortRequest_clicked()
{
	QString typeRequest;
	
	if(this->authenticationIsRunning){
		typeRequest = "Authentication";
	}
	else{
		typeRequest = "Request";
	}
	
    if(Util::Dialogs::showQuestion(this,"Are you sure you want to abort the current " + typeRequest + "?")){
        if(this->currentReply.has_value()){
            this->currentReply.value()->abort();
            Util::StatusBar::showError(ui->statusBar, typeRequest + " was aborted.");
        }
    }
}

void MainWindow::setProjectHasChanged(){

    if(!this->ignoreAnyChangesToProject.ConditionIsSet() && !this->unsavedChangesExist){
        this->unsavedChangesExist = true;

        updateWindowTitle();
    }
}

QMessageBox::StandardButton MainWindow::askToSaveCurrentProject(){
    QMessageBox::StandardButton result =
            Util::Dialogs::showQuestionWithCancel(this,"There are unsaved changes. Do you want to save the current project?", QMessageBox::StandardButton::Yes);

    if(result == QMessageBox::StandardButton::Yes){
        on_actionSave_Project_triggered();
    }

    return result;
}

void MainWindow::on_tbCopyToClipboardRequest_clicked()
{
    QString textToClipboard;

    auto fRequestFormToString = [](const QVector<UtilFRequest::HttpFormKeyValueType> &requestForm){

        QString result;

        for(int i=0; i < requestForm.size(); i++){
            result = requestForm[i].key + ": " + requestForm[i].value;

            if(i != requestForm.size()-1){
                result += "\n";
            }
        }

        return result;
    };

    auto fRequestHeadersToString = [](const QVector<UtilFRequest::HttpHeader> &requestHeaders){

        QString result;

        for(int i=0; i < requestHeaders.size(); i++){
            result = requestHeaders[i].name + ": " + requestHeaders[i].value;

            if(i != requestHeaders.size()-1){
                result += "\n";
            }
        }

        return result;
    };
	
    if(ui->twRequest->tabText(ui->twRequest->currentIndex()) == "Body"){
        switch(UtilFRequest::getBodyTypeByString(ui->cbBodyType->currentText())){
        case UtilFRequest::BodyType::RAW:
        {
            textToClipboard = ui->pteRequestBody->toPlainText();
            break;
        }
            break;
        case UtilFRequest::BodyType::FORM_DATA:
        {
            textToClipboard = fRequestFormToString(getRequestForm());
            break;
        }
            break;
        case UtilFRequest::BodyType::X_FORM_WWW_URLENCODED:
        {
            textToClipboard = fRequestFormToString(getRequestForm());
            break;
        }
        default:
        {
            QString errorMessage = "Invalid body type " + QString::number(static_cast<int>(UtilFRequest::getBodyTypeByString(ui->cbBodyType->currentText()))) + "'. Program can't proceed.";
            Util::Dialogs::showError(errorMessage);
            LOG_FATAL << errorMessage;
            exit(1);
        }
        }
    }
    else if(ui->twRequest->tabText(ui->twRequest->currentIndex()) == "Headers"){
        textToClipboard = fRequestHeadersToString(getRequestHeaders());
    }

    QApplication::clipboard()->setText(textToClipboard);
}

void MainWindow::on_tbCopyToClipboardResponse_clicked()
{
    QString textToClipboard;

    if(ui->twResponse->tabText(ui->twResponse->currentIndex()) == "Body"){
        textToClipboard = ui->pteResponseBody->toPlainText();
    }
    else if(ui->twResponse->tabText(ui->twResponse->currentIndex()) == "Headers"){
        textToClipboard = ui->pteResponseHeaders->toPlainText();
    }

    QApplication::clipboard()->setText(textToClipboard);
}

void MainWindow::on_actionPreferences_triggered()
{
    //Show preferences
    Preferences *preferencesWindow = new Preferences(this, this->currentSettings);
    // this is disconnected automatically once preferences object is destroyed:
    // https://stackoverflow.com/a/9264888/1499019
    connect(preferencesWindow, SIGNAL(saveSettings()), this, SLOT(saveCurrentSettings()), Qt::ConnectionType::DirectConnection);
    preferencesWindow->exec(); //it destroys itself when finished.
}

void MainWindow::saveCurrentSettings(){
    this->configFileManager.saveSettings(this->currentSettings);
}

void MainWindow::addDefaultHeaders(){

    UtilFRequest::RequestType currentRequestType = UtilFRequest::getRequestTypeByString(ui->cbRequestType->currentText());


    std::experimental::optional<ConfigFileFRequest::ProtocolHeader>& currentProtocolHeader = ConfigFileFRequest::getSettingsHeaderForRequestType(currentRequestType, this->currentSettings);

    if(currentProtocolHeader.has_value()){
        std::experimental::optional<QVector<UtilFRequest::HttpHeader>> *currentHeaders = nullptr;

        switch(UtilFRequest::getBodyTypeByString(ui->cbBodyType->currentText())){
        case UtilFRequest::BodyType::RAW:
        {
            currentHeaders = &currentProtocolHeader.value().headers_Raw;
            break;
        }
            break;
        case UtilFRequest::BodyType::FORM_DATA:
        {
            currentHeaders = &currentProtocolHeader.value().headers_Form_Data;
            break;
        }
            break;
        case UtilFRequest::BodyType::X_FORM_WWW_URLENCODED:
        {
            currentHeaders = &currentProtocolHeader.value().headers_X_form_www_urlencoded;
            break;
        }
        default:
        {
            QString errorMessage = "Invalid body type " + QString::number(static_cast<int>(UtilFRequest::getBodyTypeByString(ui->cbBodyType->currentText()))) + "'. Program can't proceed.";
            Util::Dialogs::showError(errorMessage);
            LOG_FATAL << errorMessage;
            exit(1);
        }
        }

        if(currentHeaders == nullptr){
            QString errorMessage = "An error ocurred while trying to insert the default headers. currentHeaders == nullptr";
            LOG_ERROR << errorMessage;
            Util::Dialogs::showError(errorMessage);
            return;
        }

        if(currentHeaders->has_value()){
            for(const UtilFRequest::HttpHeader &currentHeader : currentHeaders->value()){
                Util::TableWidget::addRow(ui->twRequestHeadersKeyValue, QStringList() << currentHeader.name << currentHeader.value);
            }
        }
    }
}

void MainWindow::setRequestType(UtilFRequest::RequestType requestType){
    ui->cbRequestType->setCurrentText(UtilFRequest::getRequestTypeString(requestType));
}

void MainWindow::formatRequestBody(const UtilFRequest::SerializationFormatType serializationType){

    this->xmlRequestBodyHighligher.setDocument(nullptr);
    this->jsonRequestBodyHighligher.setDocument(nullptr);

    if(serializationType != UtilFRequest::SerializationFormatType::UNKNOWN){

        switch(serializationType){
        case UtilFRequest::SerializationFormatType::JSON:
        {
            this->jsonRequestBodyHighligher.setDocument(ui->pteRequestBody->document());

            // we re-highlight immediately because we may want to ignore the change in the plain text edit
            // (is just formatting or re-color), this way frequest doesn't think the project was changed
            this->jsonRequestBodyHighligher.rehighlight();
            break;
        }
        case UtilFRequest::SerializationFormatType::XML:
        {
            this->xmlRequestBodyHighligher.setDocument(ui->pteRequestBody->document());

            // we re-highlight immediately because we may want to ignore the change in the plain text edit
            // (is just formatting or re-color), this way frequest doesn't think the project was changed
            this->xmlRequestBodyHighligher.rehighlight();
            break;
        }
        default:
        {
            QString errorMessage = "Invalid serializationType " + QString::number(static_cast<int>(serializationType)) + "'. Program can't proceed.";
            Util::Dialogs::showError(errorMessage);
            LOG_FATAL << errorMessage;
            exit(1);
        }
        }
    }

}

void MainWindow::formatResponseBody(const UtilFRequest::SerializationFormatType serializationType){

    this->xmlResponseBodyHighligher.setDocument(nullptr);
    this->jsonResponseBodyHighligher.setDocument(nullptr);

    if(serializationType != UtilFRequest::SerializationFormatType::UNKNOWN){

        switch(serializationType){
        case UtilFRequest::SerializationFormatType::JSON:
        {
            this->jsonResponseBodyHighligher.setDocument(ui->pteResponseBody->document());
            break;
        }
        case UtilFRequest::SerializationFormatType::XML:
        {
            this->xmlResponseBodyHighligher.setDocument(ui->pteResponseBody->document());
            break;
        }
        default:
        {
            QString errorMessage = "Invalid serializationType " + QString::number(static_cast<int>(serializationType)) + "'. Program can't proceed.";
            Util::Dialogs::showError(errorMessage);
            LOG_FATAL << errorMessage;
            exit(1);
        }
        }
    }

}

bool MainWindow::formKeyValueInBodyIsValid(){
    for(int i = 0; i < ui->twRequestBodyKeyValue->rowCount(); i++){

        // const QString &currKey = ui->twRequestBodyKeyValue->item(i,0)->text(); // ignore
        const QString &currValue = ui->twRequestBodyKeyValue->item(i,1)->text();
        const UtilFRequest::FormKeyValueType currFormKeyValueType = UtilFRequest::getFormKeyTypeByString(ui->twRequestBodyKeyValue->item(i,2)->text());

        if(currFormKeyValueType == UtilFRequest::FormKeyValueType::FILE){
            if(!QFile::exists(currValue)){
                QString errorMessage = "File '" + currValue + "' doesn't exist.\n\nPlease fix it in your request body form data and try again.";
                Util::Dialogs::showError(errorMessage);
                LOG_ERROR << errorMessage;
                return false;
            }
        }
    }

    return true;
}

bool MainWindow::formKeyValueInBodyHasFiles(){
    for(int i = 0; i < ui->twRequestBodyKeyValue->rowCount(); i++){

        const UtilFRequest::FormKeyValueType currFormKeyValueType = UtilFRequest::getFormKeyTypeByString(ui->twRequestBodyKeyValue->item(i,2)->text());

        if(currFormKeyValueType == UtilFRequest::FormKeyValueType::FILE){
            return true;
        }
    }

    return false;
}

void MainWindow::removeAllFilesRowsFromFormKeyValueInBody(){
    for(int i = 0; i < ui->twRequestBodyKeyValue->rowCount(); i++){

        const UtilFRequest::FormKeyValueType currFormKeyValueType = UtilFRequest::getFormKeyTypeByString(ui->twRequestBodyKeyValue->item(i,2)->text());

        if(currFormKeyValueType == UtilFRequest::FormKeyValueType::FILE){
            ui->twRequestBodyKeyValue->selectRow(i);
        }
    }

    Util::TableWidget::deleteSelectedRows(ui->twRequestBodyKeyValue);
}

UtilFRequest::SerializationFormatType MainWindow::getRequestCurrentSerializationFormatType(){
    UtilFRequest::SerializationFormatType serializationType = UtilFRequest::SerializationFormatType::UNKNOWN;

    if(ui->actionFormat_Request_body->isChecked()){
        serializationType = UtilFRequest::getSerializationFormatTypeForString(ui->pteRequestBody->toPlainText());
    }

    return serializationType;
}

UtilFRequest::SerializationFormatType MainWindow::getResponseCurrentSerializationFormatType(){
    UtilFRequest::SerializationFormatType serializationType = UtilFRequest::SerializationFormatType::UNKNOWN;

    if(ui->actionFormat_Response_Body->isChecked()){
        serializationType = UtilFRequest::getSerializationFormatTypeForString(ui->pteResponseBody->toPlainText());
    }

    return serializationType;
}

QString MainWindow::getNewUuid(){
    QString generatedUuid;

    // make sure we get a unique identifier (while very small, there it is still possible to exist collisions,
    // plus the xml uuid can be edited by hand by anyone)
    do
    {
        generatedUuid = QUuid::createUuid().toString();
    } while( this->uuidsInUse.contains(generatedUuid) );

    this->uuidsInUse.insert(generatedUuid);

    return generatedUuid;
}

void MainWindow::saveProjectProperties(){

    // Project properties changed, we need to save the project file and maybe the configuration file (for the authentications)
    this->unsavedChangesExist = true;

    updateWindowTitle();

    reloadRequest(this->currentItem);

    on_actionSave_Project_triggered();

    // Do we have auth data to save?
    if(this->currentProjectItem->authData != nullptr){

        // Assume the credentials are new, so a new authentication may need to be done
        this->currentProjectAuthenticationWasMade = false;

        // Is to save to config file?
        if(this->currentProjectItem->authData->saveAuthToConfigFile){

            ConfigFileFRequest::ConfigurationProjectAuthentication currConfigProjAuth;
            currConfigProjAuth.lastProjectName = this->currentProjectItem->projectName;
            currConfigProjAuth.projectUuid = this->currentProjectItem->getUuid();
            currConfigProjAuth.authData = this->currentProjectItem->authData;

            this->currentSettings.mapOfConfigAuths_UuidToConfigAuth[this->currentProjectItem->getUuid()] =
                    currConfigProjAuth; // add or override config proj auth data

            saveCurrentSettings();
        }
        else{ // Nop! We are saving to project file. In this case make sure we remove the auth data from config file if it exists
            if(this->currentSettings.mapOfConfigAuths_UuidToConfigAuth.contains(this->currentProjectItem->getUuid())){
                this->currentSettings.mapOfConfigAuths_UuidToConfigAuth.remove(this->currentProjectItem->getUuid());
                saveCurrentSettings();
            }
        }


    }

    if(!this->unsavedChangesExist){
        Util::Dialogs::showInfo("Project properties saved with success!");
    }
    else{
        Util::Dialogs::showWarning("Project properties weren't saved. Please save the project manually from file menu.");
    }
}

void MainWindow::on_leRequestsFilter_textChanged(const QString &arg1)
{
    QString trimmedFilter = arg1.trimmed();
	
	QPalette palette; // to set filter background color
	
	if(trimmedFilter.isEmpty()){ // if filter is empty
		ui->treeWidget->headerItem()->setText(0, "Requests");
		palette.setColor(QPalette::Base, ui->lePath->palette().color(QPalette::Base)); // just use another text box to get the default value
		ui->leRequestsFilter->setPalette(palette);
	
		if(this->currentProjectItem != nullptr){
			for(int i=0; i<this->currentProjectItem->childCount(); i++){
				this->currentProjectItem->child(i)->setHidden(false);
			}
		}
	}
	else{
		ui->treeWidget->headerItem()->setText(0, "Requests (filtred)");
		palette.setColor(QPalette::Base, 0xFFFACD /* LemonChiffon */);
		ui->leRequestsFilter->setPalette(palette);
		
		// Show only the ones which the name match the filter
		if(this->currentProjectItem != nullptr){
			for(int i=0; i<this->currentProjectItem->childCount(); i++){
				if(!this->currentProjectItem->child(i)->text(0).contains(trimmedFilter, Qt::CaseInsensitive)){ // TODO change with enum
					this->currentProjectItem->child(i)->setHidden(true);
				}
				else{
					this->currentProjectItem->child(i)->setHidden(false);
				}		
			}
		}
	}
}

void MainWindow::openProjectProperties(){
	
	// Show project properties
	ProjectProperties *projectPropertiesDialog = new ProjectProperties(this, this->currentProjectItem);

	// https://stackoverflow.com/a/9264888/1499019
	connect(projectPropertiesDialog, SIGNAL (signalSaveProjectProperties()), this, SLOT(saveProjectProperties()));
	projectPropertiesDialog->exec(); //it destroys itself when finished.
	
}

void MainWindow::on_actionProject_Properties_triggered()
{
    openProjectProperties();
}

void MainWindow::removeRequest(FRequestTreeWidgetRequestItem * const itemToDelete){
	
        if(Util::Dialogs::showQuestion(this, "Remove the request '" + itemToDelete->text(0) + "'?")){

            QString uuidToRemove = itemToDelete->itemContent.uuid;

            this->currentProjectItem->removeChild(itemToDelete);

            this->uuidsToCleanUp.append(uuidToRemove);
            this->uuidsInUse.remove(uuidToRemove);

            if(ui->treeWidget->currentItem() != this->currentProjectItem){
                this->currentItem = FRequestTreeWidgetRequestItem::fromQTreeWidgetItem(ui->treeWidget->currentItem());
            }
            else{
                this->currentItem = nullptr;
            }

            // We need to call this event manually when a item is removed (it is not called automatically in this case)
            on_treeWidget_currentItemChanged(ui->treeWidget->currentItem(), nullptr);

            setProjectHasChanged();
        }
}

void MainWindow::treeWidgetDeleteKeyPressed(){
	
    if (this->currentItem != nullptr && ui->treeWidget->currentItem() == this->currentItem)
    {
        removeRequest(this->currentItem);
    }
	
}