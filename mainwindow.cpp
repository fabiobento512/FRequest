/*
 *
Copyright (C) 2017  Fábio Bento (random-guy)

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
    ui->setupUi(this);

    // Disable drop of items outside of the project folder
    // Thanks to "p.i.g.": http://stackoverflow.com/a/30580654
    ui->treeWidget->invisibleRootItem()->setFlags(
                ui->treeWidget->invisibleRootItem()->flags() ^ Qt::ItemIsDropEnabled
                );

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

    // json highlighters
    setFormatRequestAndResponseBodies();

    loadRecentProjects();

    // Hide key value table for now (the raw textedit is displayed by default)
    ui->twRequestBodyKeyValue->hide();
    ui->tbRequestBodyKeyValueAdd->hide();
    ui->tbRequestBodyKeyValueRemove->hide();

    // We will also set their size to the normal size (we start with size 0 so vertical splitter divide the area in equal space (for header and response), kinda hacky)
    ui->twRequestBodyKeyValue->setMaximumSize(this->auxMaximumSize);
    ui->tbRequestBodyKeyValueAdd->setMaximumSize(this->auxMaximumSize);
    ui->tbRequestBodyKeyValueRemove->setMaximumSize(this->auxMaximumSize);

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

    this->ignoreAnyChangesToProject = false;

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
    // Disable until this request is finished
    ui->pbSendRequest->setEnabled(false);

    // Clear previous request data:
    clearOlderResponse();

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));

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
            manager->setProxy(proxy);
        }
    }
    else{
        QNetworkProxy proxy;
        proxy.setType(QNetworkProxy::NoProxy);
        QNetworkProxy::setApplicationProxy(proxy);
    }

    QNetworkReply *reply;

    reply = processHttpRequest(manager, UtilFRequest::getRequestTypeByText(ui->cbRequestType->currentText()));

    lastStartTime = QDateTime::currentDateTime();

    checkForQNetworkAccessManagerTimeout(reply);

    reply->deleteLater();
    manager->deleteLater();
}

// Since QNetworkReply doesn't have a way to set a timeout we need implement it by ourselves
// http://stackoverflow.com/a/13229926
void MainWindow::checkForQNetworkAccessManagerTimeout(QNetworkReply *reply)
{
    // Since QNetworkReply doesn't have a way to set a timeout we need implement it by ourselves
    // http://stackoverflow.com/a/13229926
    QTimer timer;
    timer.setSingleShot(true);

    QEventLoop loop;
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    timer.start(this->currentSettings.requestTimeout * 1000);
    loop.exec();

    if(timer.isActive())
    {
        timer.stop();
    }
    else
    {
        // timeout
        ui->lbStatus->setText("-1");
        ui->lbDescription->setText("Timeout after " + QString::number(this->currentSettings.requestTimeout) + " seconds");
        ui->lbTimeElapsed->setText(QString::number(lastStartTime.msecsTo(QDateTime::currentDateTime())) + " ms");

        this->lastReplyStatusError = -1;

        disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));

        reply->abort();
    }
}

void MainWindow::replyFinished(QNetworkReply *reply){

    // -1 means we have set a custom error, we don't want to override that one
    if(this->lastReplyStatusError != -1 && reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).isValid()){
        ui->lbStatus->setText(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString());
        ui->lbDescription->setText(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString());
    }
    else if(this->lastReplyStatusError == 0 && reply->error() != QNetworkReply::NoError){
        ui->lbStatus->setText("N/A");
        ui->lbDescription->setText(reply->errorString() + " - Error " + QString::number(reply->error()));
    }

    ui->lbTimeElapsed->setText(QString::number(lastStartTime.msecsTo(QDateTime::currentDateTime())) + " ms");

    if(reply->error() == QNetworkReply::NoError)
    {
        QString headersText;

        for(const QNetworkReply::RawHeaderPair &currentPair : reply->rawHeaderPairs()){
            headersText += currentPair.first + ": " + currentPair.second + "\n";
        }

        QByteArray responseData = reply->readAll();

        ui->pteResponseHeaders->document()->setPlainText(headersText);

        if(ui->actionFormat_Response_Body->isChecked()){

            // Try to parse the response as json, if it fails show the raw data instead

            QJsonParseError parseError;

            QJsonDocument auxJsonDoc = QJsonDocument::fromJson(responseData, &parseError);


            if(parseError.error == QJsonParseError::NoError){

                ui->pteResponseBody->document()->setPlainText(auxJsonDoc.toJson());

            }

            else{

                ui->pteResponseBody->document()->setPlainText(responseData);

            }

        }
        else{
            ui->pteResponseBody->document()->setPlainText(responseData);
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
                    file.write(responseData);
                    file.close();

                    if(ui->actionOpen_file_after_download->isChecked()){
                        if(!QDesktopServices::openUrl("file:///"+filePath)){
                            QString errorMessage = "Could not open downloaded file: " + filePath;
                            Util::Dialogs::showError(errorMessage);
                            LOG_ERROR << errorMessage;
                            Util::StatusBar::showError(ui->statusBar, errorMessage);
                        }
                    }

                    Util::StatusBar::showSuccess(ui->statusBar, "File saved with sucess.");
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
            Util::StatusBar::showSuccess(ui->statusBar, "Request performed with sucess.");
        }
    }
    else{

        QString errorMessage = ui->lbDescription->text();

        LOG_ERROR << errorMessage;

        Util::Dialogs::showError("An error occurred while performing the request.\n" + errorMessage);
        Util::StatusBar::showError(ui->statusBar, "Request was not performed with sucess.");
    }

    ui->pbSendRequest->setEnabled(true);
}

void MainWindow::on_leMainUrl_textChanged(const QString &)
{
    setProjectHasChanged();

    if(!ui->cbRequestOverrideMainUrl->isChecked()){
        buildFullPath();
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
        normalizedMainUrl = ui->leMainUrl->text();
    }

    if(!normalizedMainUrl.endsWith('/') && !ui->lePath->text().startsWith('/')){
        normalizedMainUrl += '/';
    }

    ui->leFullPath->setText(normalizedMainUrl + ui->lePath->text());
}

void MainWindow::on_lePath_textChanged(const QString &)
{
    setProjectHasChanged();

    buildFullPath();
}

QNetworkReply* MainWindow::processHttpRequest(QNetworkAccessManager * const manager, UtilFRequest::RequestType requestType)
{
    std::unique_ptr<HttpRequest> httpRequest = nullptr;

    QString fullPath = ui->leFullPath->text();
    QString bodyType = ui->cbBodyType->currentText();
    QString requestBody = ui->pteRequestBody->toPlainText();
    QVector<UtilFRequest::HttpHeader> requestHeaders = getRequestHeaders();

    switch(requestType){
    case UtilFRequest::RequestType::GET_OPTION:
    {
        httpRequest = std::make_unique<GetHttpRequest>(manager, fullPath, requestHeaders);
        break;
    }
    case UtilFRequest::RequestType::POST_OPTION:
    {
        httpRequest = std::make_unique<PostHttpRequest>(manager, ui->twRequestBodyKeyValue, fullPath , bodyType, requestBody, requestHeaders);
        break;
    }
    case UtilFRequest::RequestType::PUT_OPTION:
    {
        httpRequest = std::make_unique<PutHttpRequest>(manager, ui->twRequestBodyKeyValue, fullPath , bodyType, requestBody, requestHeaders);
        break;
    }
    case UtilFRequest::RequestType::DELETE_OPTION:
    {
        httpRequest = std::make_unique<DeleteHttpRequest>(manager, fullPath, requestHeaders);
        break;
    }
    case UtilFRequest::RequestType::PATCH_OPTION:
    {
        httpRequest = std::make_unique<PatchHttpRequest>(manager, ui->twRequestBodyKeyValue, fullPath , bodyType, requestBody, requestHeaders);
        break;
    }
    case UtilFRequest::RequestType::HEAD_OPTION:
    {
        httpRequest = std::make_unique<HeadHttpRequest>(manager, fullPath, requestHeaders);
        break;
    }
    case UtilFRequest::RequestType::TRACE_OPTION:
    {
        httpRequest = std::make_unique<TraceHttpRequest>(manager, fullPath, requestHeaders);
        break;
    }
    case UtilFRequest::RequestType::OPTIONS_OPTION:
    {
        httpRequest = std::make_unique<OptionsHttpRequest>(manager, ui->twRequestBodyKeyValue, fullPath , bodyType, requestBody, requestHeaders);
        break;
    }
    default:{
        QString errorMessage = "Request type unknown: '" + ui->cbRequestType->currentText() + "'. Program can't proceed.";
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

    foreach(QModelIndex rowItem, myTree->selectionModel()->selectedRows()){
        selectedRows << rowItem.row();
    }

    std::unique_ptr<QMenu> menu = std::make_unique<QMenu>();
    std::unique_ptr<QAction> openProjectLocation = nullptr;

    if(ui->treeWidget->currentItem() == this->currentProjectItem){
        openProjectLocation = std::make_unique<QAction>("Open project location", myTree);
        menu->addAction(openProjectLocation.get());
        menu->addSeparator();
    }

    std::unique_ptr<QAction> addNewRequest =  std::make_unique<QAction>("Add new request", myTree);
    menu->addAction(addNewRequest.get());
    std::unique_ptr<QAction> cloneRequest =  std::make_unique<QAction>("Clone request", myTree);
    menu->addAction(cloneRequest.get());
    menu->addSeparator();
    std::unique_ptr<QAction> moveRequestUp =  std::make_unique<QAction>("Move up", myTree);
    menu->addAction(moveRequestUp.get());
    std::unique_ptr<QAction> moveRequestDown =  std::make_unique<QAction>("Move down", myTree);
    menu->addAction(moveRequestDown.get());
    menu->addSeparator();
    std::unique_ptr<QAction> deleteRequest =  std::make_unique<QAction>("Delete request", myTree);
    menu->addAction(deleteRequest.get());

    // Necessary to got to treeWidget->currentItem because our global variable doesn't get updated for the project item
    if(ui->treeWidget->currentItem() == this->currentProjectItem){
        deleteRequest->setEnabled(false);
        moveRequestDown->setEnabled(false);
        moveRequestUp->setEnabled(false);
        cloneRequest->setEnabled(false);
    }
    else if(this->currentProjectItem->childCount() == 1){
        moveRequestDown->setEnabled(false);
        moveRequestUp->setEnabled(false);
    }
    else if(ui->treeWidget->itemAbove(ui->treeWidget->currentItem()) == this->currentProjectItem){
        moveRequestUp->setEnabled(false);
    }
    else if(ui->treeWidget->itemBelow(ui->treeWidget->currentItem()) == nullptr){
        moveRequestDown->setEnabled(false);
    }

    // Disable show in explorer if we don't have any project saved to disk
    if(openProjectLocation != nullptr && this->lastProjectFilePath.isEmpty()){
        openProjectLocation->setEnabled(false);
    }

    QAction* selectedOption = menu->exec(myTree->viewport()->mapToGlobal(pos));

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

        this->ignoreAnyChangesToProject = true;
        this->unsavedChangesExist = true;

        FRequestTreeWidgetItem *newRequest = addRequestItem("New Request", this->currentProjectItem);
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

        this->ignoreAnyChangesToProject = false;
    }
    else if(selectedOption == cloneRequest.get()){
        this->ignoreAnyChangesToProject = true;
        this->unsavedChangesExist = true;

        const FRequestTreeWidgetItem * const itemToClone = FRequestTreeWidgetItem::fromQTreeWidgetItem(ui->treeWidget->currentItem());

        FRequestTreeWidgetItem *newRequest = addRequestItem(itemToClone->text(0), this->currentProjectItem);
        ui->treeWidget->clearSelection();
        newRequest->setSelected(true);

        // Necessary in order to currentIndexChanged to work correctly (select it is not enough)
        ui->treeWidget->setCurrentItem(newRequest);

        this->currentItem = newRequest;

        this->ignoreAnyChangesToProject = false;
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

        if(Util::Dialogs::showQuestion(this, "Remove the request '" + this->currentItem->text(0) + "'?")){

            QString uuidToRemove = this->currentItem->itemContent.uuid;

            this->currentProjectItem->removeChild(this->currentItem);
            this->uuidsToCleanUp.append(uuidToRemove);
            this->uuidsInUse.remove(uuidToRemove);

            if(ui->treeWidget->currentItem() != this->currentProjectItem){
                this->currentItem = FRequestTreeWidgetItem::fromQTreeWidgetItem(ui->treeWidget->currentItem());
            }
            else{
                this->currentItem = nullptr;
            }

            setProjectHasChanged();
        }
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
    }

    if(previous != nullptr){
        previousFRequestItem = FRequestTreeWidgetItem::fromQTreeWidgetItem(previous);
    }

    // Save previous item
    if(previousFRequestItem != nullptr && previousFRequestItem != this->currentProjectItem)
    {
        updateTreeWidgetItemContent(previousFRequestItem);
    }

    // Update window for new item
    if(currentFRequestItem != nullptr){

        if(!currentFRequestItem->isProjectItem){

            this->currentItem = currentFRequestItem;

            if(!this->ignoreAnyChangesToProject){
                reloadRequest(currentFRequestItem);
            }

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
    if(item->text(0) != newItem->itemContent.name){

        if(!newItem->isProjectItem){
            updateTreeWidgetItemContent(newItem);
        }

        updateInterface = true;
    }
    else if(newItem->isProjectItem){ // if it is the project, we must update the interface
        updateInterface = true;
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

    if(this->currentProjectItem != nullptr){
        fRequestTitle = fRequestTitle % " - " % this->currentProjectItem->text(0);
    }

    if(this->currentItem != nullptr){
        fRequestTitle = fRequestTitle % "/" % this->currentItem->text(0);
    }

    fRequestTitle = fRequestTitle % " - " % GlobalVars::AppName % " " % GlobalVars::AppVersion;

    if(this->unsavedChangesExist){
        fRequestTitle = fRequestTitle % "*";
    }

    setWindowTitle(fRequestTitle);
}

FRequestTreeWidgetItem* MainWindow::addProjectItem(const QString &projectName){
    FRequestTreeWidgetItem *projectFolder = new FRequestTreeWidgetItem(QStringList() << projectName, true);
    projectFolder->setIcon(0, QIcon(":/icons/projects_folder_icon.png"));
    projectFolder->setToolTip(0, projectFolder->text(0));
    projectFolder->setFlags(projectFolder->flags() | Qt::ItemIsEditable);
    ui->treeWidget->addTopLevelItem(projectFolder);
    projectFolder->setExpanded(true);

    return projectFolder;
}

FRequestTreeWidgetItem* MainWindow::addRequestItem(const QString &requestName, FRequestTreeWidgetItem * const currentProject){
    FRequestTreeWidgetItem *newRequest = new FRequestTreeWidgetItem(QStringList() << requestName);
    newRequest->setToolTip(0, newRequest->text(0));
    newRequest->setFlags((newRequest->flags() | Qt::ItemIsEditable) ^ Qt::ItemIsDropEnabled); // don't allow drop inside another items
    currentProject->addChild(newRequest);

    return newRequest;
}

void MainWindow::setNewProject(){

    if(this->unsavedChangesExist){
        QMessageBox::StandardButton result = askToSaveCurrentProject();
        if(result == QMessageBox::StandardButton::Cancel){
            return;
        }
    }

    clearEverything();

    FRequestTreeWidgetItem *projectFolder = addProjectItem("New Project");
    FRequestTreeWidgetItem *newRequest = addRequestItem("New Request", projectFolder);

    newRequest->setSelected(true);

    this->currentProjectItem = projectFolder;
    this->currentItem = newRequest;

    // Necessary in order to currentIndexChanged to work correctly (select it is not enough)
    ui->treeWidget->setCurrentItem(newRequest);

    this->lastProjectFilePath = QString();

    if(this->currentSettings.defaultHeaders.useDefaultHeaders){
        addDefaultHeaders();
    }

    this->unsavedChangesExist = false;
    updateWindowTitle(); // it doesn't get called automatically here
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

QVector<UtilFRequest::HttpFormKeyValue> MainWindow::getRequestForm(){

    QVector<UtilFRequest::HttpFormKeyValue> requestForm;

    for(int i = 0; i < ui->twRequestBodyKeyValue->rowCount(); i++){

        UtilFRequest::HttpFormKeyValue currentFormKeyValue;

        currentFormKeyValue.key = ui->twRequestBodyKeyValue->item(i, 0)->text();
        currentFormKeyValue.value = ui->twRequestBodyKeyValue->item(i, 1)->text();

        requestForm.append(currentFormKeyValue);
    }

    return requestForm;
}


void MainWindow::updateTreeWidgetItemContent(FRequestTreeWidgetItem * const requestItem){
    requestItem->itemContent.name = requestItem->text(0);
    requestItem->itemContent.path = ui->lePath->text();

    requestItem->itemContent.bOverridesMainUrl = ui->cbRequestOverrideMainUrl->isChecked();
    requestItem->itemContent.overrideMainUrl = ui->leRequestOverrideMainUrl->text();

    requestItem->itemContent.headers = getRequestHeaders();

    // if it is a new request get a uuid for it
    if(!requestItem->hasUuid()){

        QString generatedUuid;

        // make sure we get a unique identifier (while very small, there it is still possible to exist collisions,
        // plus the xml uuid can be edited by hand by anyone)
        do
        {
            generatedUuid = QUuid::createUuid().toString();
        } while( this->uuidsInUse.contains(generatedUuid) );

        this->uuidsInUse.insert(generatedUuid);
        requestItem->itemContent.uuid = generatedUuid;
    }

    requestItem->itemContent.order = requestItem->parent()->indexOfChild(requestItem);

    if(ui->cbBodyType->currentText() == "raw"){
        requestItem->itemContent.bodyType = UtilFRequest::BodyType::RAW;
        requestItem->itemContent.body = ui->pteRequestBody->toPlainText();
    }
    else if(ui->cbBodyType->currentText() == "form-data"){
        requestItem->itemContent.bodyType = UtilFRequest::BodyType::FORM_DATA;
        requestItem->itemContent.bodyForm = getRequestForm();
    }
    else if(ui->cbBodyType->currentText() == "x-form-www-urlencoded"){
        requestItem->itemContent.bodyType = UtilFRequest::BodyType::X_FORM_WWW_URLENCODED;
        requestItem->itemContent.bodyForm = getRequestForm();
    }

    requestItem->itemContent.requestType = UtilFRequest::getRequestTypeByText(ui->cbRequestType->currentText());

    requestItem->itemContent.bDownloadResponseAsFile = ui->cbDownloadResponseAsFile->isChecked();
}

void MainWindow::saveProjectState(const QString &filePath)
{

    // Make sure the current request is updated in memory
    if(this->currentItem != nullptr){
        updateTreeWidgetItemContent(this->currentItem);
    }

    pugi::xml_document doc;
    pugi::xml_node rootNode;

    bool isNewFile = !QFileInfo(filePath).exists();

    // If file already exists try to read the project file
    if(!isNewFile){
        if(!loadAndValidateFRequestProjectFile(filePath, doc)){
            QString errorMessage = "Couldn't load project file. Save aborted.";
            LOG_ERROR << errorMessage;
            Util::Dialogs::showError(errorMessage);
            Util::StatusBar::showError(ui->statusBar, "Couldn't save project file.");
            return;
        }

        // Try to clear deleted items from projects file if they exist
        for(const QString &currentDeletedItemUuid : this->uuidsToCleanUp){
            pugi::xml_node nodeToDelete = doc.select_node(QSTR_TO_CSTR("/FRequestProject/Request[@uuid='" + currentDeletedItemUuid + "']")).node();

            if(!nodeToDelete.empty()){
                nodeToDelete.parent().remove_child(nodeToDelete);
            }
        }

        rootNode = doc.child("FRequestProject");
    }
    else{
        rootNode = doc.append_child("FRequestProject"); // create
    }

    createOrGetPugiXmlAttribute(rootNode, "frequestVersion").set_value(QSTR_TO_CSTR(GlobalVars::LastCompatibleVersion));
    createOrGetPugiXmlAttribute(rootNode, "name").set_value(QSTR_TO_CSTR(this->currentProjectItem->text(0)));
    createOrGetPugiXmlAttribute(rootNode, "mainUrl").set_value(QSTR_TO_CSTR(ui->leMainUrl->text()));

    // Save by the current tree order
    for(int i=0; i<this->currentProjectItem->childCount(); i++){

        FRequestTreeWidgetItem::RequestInfo &currentRequest = FRequestTreeWidgetItem::fromQTreeWidgetItem(this->currentProjectItem->child(i))->itemContent;

        currentRequest.order = i;

        pugi::xml_node requestNode;

        pugi::xpath_node xpathRequestNode = doc.select_node(QSTR_TO_CSTR("/FRequestProject/Request[@uuid='" + currentRequest.uuid + "']"));

        // if it doesn't exist yet create it
        if(xpathRequestNode.node().empty()){
            requestNode = rootNode.append_child("Request");
        }
        else{ // otherwise use the existing one
            requestNode = xpathRequestNode.node();
        }

        createOrGetPugiXmlAttribute(requestNode, "name").set_value(QSTR_TO_CSTR(currentRequest.name));
        createOrGetPugiXmlAttribute(requestNode, "uuid").set_value(QSTR_TO_CSTR(currentRequest.uuid));
        createOrGetPugiXmlAttribute(requestNode, "order").set_value(i);
        createOrGetPugiXmlAttribute(requestNode, "bOverridesMainUrl").set_value(currentRequest.bOverridesMainUrl);
        createOrGetPugiXmlAttribute(requestNode, "overrideMainUrl").set_value(QSTR_TO_CSTR(currentRequest.overrideMainUrl));
        createOrGetPugiXmlAttribute(requestNode, "path").set_value(QSTR_TO_CSTR(currentRequest.path));
        createOrGetPugiXmlAttribute(requestNode, "type").set_value(static_cast<int>(currentRequest.requestType));
        createOrGetPugiXmlAttribute(requestNode, "bDownloadResponseAsFile").set_value(currentRequest.bDownloadResponseAsFile);

        // remove body if exists, we will rebuild it
        requestNode.remove_child("Body");

        pugi::xml_node bodyNode = requestNode.append_child("Body");

        bodyNode.append_attribute("type").set_value(static_cast<int>(currentRequest.bodyType));

        // Aux lambda to avoid duplicated code for FORM_DATA / X_FORM_WWW_URLENCODED
        auto fSaveFormKeyValues = [](const QVector<UtilFRequest::HttpFormKeyValue> &bodyForm, pugi::xml_node &bodyNode){
            for(const UtilFRequest::HttpFormKeyValue &currentKeyValue : bodyForm){
                pugi::xml_node currentFormKeyValueNode = bodyNode.append_child("FormKeyValue");

                currentFormKeyValueNode.append_child("Key").append_child(pugi::xml_node_type::node_cdata).set_value(QSTR_TO_CSTR(currentKeyValue.key));
                currentFormKeyValueNode.append_child("Value").append_child(pugi::xml_node_type::node_cdata).set_value(QSTR_TO_CSTR(currentKeyValue.value));
            }
        };

        switch (currentRequest.bodyType) {
        case UtilFRequest::BodyType::RAW:
        {
            bodyNode.append_child(pugi::xml_node_type::node_cdata).set_value(QSTR_TO_CSTR(currentRequest.body));
        }
            break;
        case UtilFRequest::BodyType::FORM_DATA:
        {
            fSaveFormKeyValues(currentRequest.bodyForm, bodyNode);
        }
            break;
        case UtilFRequest::BodyType::X_FORM_WWW_URLENCODED:
        {
            fSaveFormKeyValues(currentRequest.bodyForm, bodyNode);
        }
            break;
        default:
            break;
        }

        // remove headers if exists, we will rebuild it
        requestNode.remove_child("Headers");

        pugi::xml_node headersNode = requestNode.append_child("Headers");

        for(const UtilFRequest::HttpHeader &currentHeader : currentRequest.headers){
            pugi::xml_node currentHeaderNode = headersNode.append_child("Header");

            currentHeaderNode.append_child("Key").append_child(pugi::xml_node_type::node_cdata).set_value(QSTR_TO_CSTR(currentHeader.name));
            currentHeaderNode.append_child("Value").append_child(pugi::xml_node_type::node_cdata).set_value(QSTR_TO_CSTR(currentHeader.value));
        }
    }

    if(!doc.save_file(filePath.toUtf8().constData(), PUGIXML_TEXT("\t"), pugi::format_default | pugi::format_write_bom, pugi::xml_encoding::encoding_utf8)){
        QString errorMessage = "An error ocurred while trying to save the project file. Please try another path.";
        LOG_ERROR << errorMessage;
        Util::Dialogs::showError(errorMessage);
        return;
    }

    this->currentSettings.lastProjectPath = QFileInfo(filePath).absoluteDir().path();

    this->lastProjectFilePath = filePath;
    this->uuidsToCleanUp.clear();
    this->unsavedChangesExist = false;
    addNewRecentProject(filePath);

    updateWindowTitle();

    Util::StatusBar::showSuccess(ui->statusBar, "Project saved with sucess!");
}

void MainWindow::loadProjectState(const QString &filePath)
{

    if(this->unsavedChangesExist){
        QMessageBox::StandardButton result = askToSaveCurrentProject();
        if(result == QMessageBox::StandardButton::Cancel){
            return;
        }
    }

    QString statusError = "Couldn't load project.";

    pugi::xml_document doc;

    if(!loadAndValidateFRequestProjectFile(filePath, doc)){
        QString errorMessage = "Couldn't load project file.";
        LOG_ERROR << errorMessage;
        Util::Dialogs::showError(errorMessage);
        Util::StatusBar::showError(ui->statusBar, "Couldn't load project.");
        return;
    }

    // After the initial validations begin loading the project data

    this->ignoreAnyChangesToProject = true;

    clearEverything();

    try{
        ui->leMainUrl->setText(doc.select_node("/FRequestProject/@mainUrl").attribute().value());

        // Add project item
        this->currentProjectItem = addProjectItem(QString(doc.select_node("/FRequestProject/@name").attribute().value()));

        // Add request items

        pugi::xpath_node_set requestNodes = doc.select_nodes("/FRequestProject/Request");

        // Aux lambda to avoid duplicated code for FORM_DATA / X_FORM_WWW_URLENCODED
        auto fGetBodyForm = [](pugi::xml_node &bodyNode) -> QVector<UtilFRequest::HttpFormKeyValue>
        {
            QVector<UtilFRequest::HttpFormKeyValue> bodyForm;

            for(const pugi::xml_node &currentFormKeyValueNode : bodyNode.children()){
                UtilFRequest::HttpFormKeyValue currentFormKeyValue;

                currentFormKeyValue.key = currentFormKeyValueNode.child("Key").child_value();
                currentFormKeyValue.value = currentFormKeyValueNode.child("Value").child_value();

                bodyForm.append(currentFormKeyValue);
            }

            return bodyForm;
        };

        QList<FRequestTreeWidgetItem::RequestInfo> loadedRequestInfo;

        for(size_t i=0; i < requestNodes.size(); i++){

            pugi::xml_node currNode = requestNodes[i].node();

            FRequestTreeWidgetItem::RequestInfo currentRequestInfo;

            currentRequestInfo.path = currNode.attribute("path").as_string();
            currentRequestInfo.requestType = static_cast<UtilFRequest::RequestType>(currNode.attribute("type").as_int());

            pugi::xml_node bodyNode = currNode.child("Body");

            currentRequestInfo.bodyType = static_cast<UtilFRequest::BodyType>(bodyNode.attribute("type").as_int());

            switch (currentRequestInfo.bodyType) {
            case UtilFRequest::BodyType::RAW:
            {
                currentRequestInfo.body = bodyNode.child_value();
            }
                break;
            case UtilFRequest::BodyType::FORM_DATA:
            {
                currentRequestInfo.bodyForm = fGetBodyForm(bodyNode);
            }
                break;
            case UtilFRequest::BodyType::X_FORM_WWW_URLENCODED:
            {
                currentRequestInfo.bodyForm = fGetBodyForm(bodyNode);
            }
                break;
            default:
                break;
            }

            QVector<UtilFRequest::HttpHeader> requestHeaders;

            for(const pugi::xml_node &currentHeaderNode : currNode.child("Headers").children()){
                UtilFRequest::HttpHeader currentHeader;

                currentHeader.name = QString(currentHeaderNode.child("Key").child_value());
                currentHeader.value = currentHeaderNode.child("Value").child_value();

                requestHeaders.append(currentHeader);
            }

            currentRequestInfo.headers = requestHeaders;

            currentRequestInfo.bOverridesMainUrl = currNode.attribute("bOverridesMainUrl").as_bool();
            currentRequestInfo.overrideMainUrl = currNode.attribute("overrideMainUrl").value();
            currentRequestInfo.bDownloadResponseAsFile = currNode.attribute("bDownloadResponseAsFile").as_bool();
            currentRequestInfo.name = requestNodes[i].node().attribute("name").value();
            currentRequestInfo.uuid = requestNodes[i].node().attribute("uuid").value();
            this->uuidsInUse.insert(currentRequestInfo.uuid);
            currentRequestInfo.order = requestNodes[i].node().attribute("order").as_ullong();

            // insert loaded request into our temp vector
            loadedRequestInfo.append(currentRequestInfo);
        }

        // Order them by the correct order
        std::sort(
                    loadedRequestInfo.begin(),
                    loadedRequestInfo.end(),
                    [](const FRequestTreeWidgetItem::RequestInfo &first, const FRequestTreeWidgetItem::RequestInfo &second){ return first.order < second.order; }
        );

        // Now the items are in the correct order, we can now load them in the interface
        for(int i=0; i<loadedRequestInfo.size(); i++){

            FRequestTreeWidgetItem::RequestInfo &currentRequestInfo = loadedRequestInfo[i];

            // fix order
            currentRequestInfo.order = i;

            FRequestTreeWidgetItem *currRequest = addRequestItem(currentRequestInfo.name, this->currentProjectItem);
            currRequest->itemContent = currentRequestInfo;

            // Load first request
            if(i==0){

                ui->lePath->setText(currentRequestInfo.path);

                setRequestType(currentRequestInfo.requestType);

                ui->pteRequestBody->setPlainText(currentRequestInfo.body);

                for(const UtilFRequest::HttpHeader &currentHeader : currentRequestInfo.headers){
                    Util::TableWidget::addRow(ui->twRequestHeadersKeyValue, QStringList() << currentHeader.name << currentHeader.value);
                }

                ui->cbDownloadResponseAsFile->setChecked(currentRequestInfo.bDownloadResponseAsFile);

                currRequest->setSelected(true);

                this->currentItem = currRequest;

                // Necessary in order to currentIndexChanged to work correctly (select it is not enough)
                ui->treeWidget->setCurrentItem(currRequest);

                updateWindowTitle(); // it doesn't get called automatically here
            }

        }
    }
    catch(const std::exception& e){
        QString errorMessage = "Couldn't load the FRequest project. Error: " + QString(e.what());
        LOG_ERROR << errorMessage;
        Util::Dialogs::showError(errorMessage);
        Util::StatusBar::showError(ui->statusBar, statusError);
        this->ignoreAnyChangesToProject = false;
        return;
    }

    this->currentSettings.lastProjectPath = QFileInfo(filePath).absoluteDir().path();

    this->lastProjectFilePath = filePath;
    this->unsavedChangesExist = false;

    this->ignoreAnyChangesToProject = false;

    addNewRecentProject(filePath);

    updateWindowTitle();

    Util::StatusBar::showSuccess(ui->statusBar, "Project loaded sucessfully.");
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
    setFormatRequestAndResponseBodies();
}

void MainWindow::on_actionFormat_Request_body_triggered()
{
    setFormatRequestAndResponseBodies();
}

void MainWindow::reloadRequest(FRequestTreeWidgetItem* const item){

    this->ignoreAnyChangesToProject = true;

    FRequestTreeWidgetItem::RequestInfo info = item->itemContent;

    // Clear old contents
    clearRequestAndResponse();

    ui->lePath->setText(info.path);
    ui->cbRequestOverrideMainUrl->setChecked(info.bOverridesMainUrl);
    ui->leRequestOverrideMainUrl->setText(info.overrideMainUrl);

    setRequestType(info.requestType);

    // Aux lambda to avoid duplicated code for FORM_DATA / X_FORM_WWW_URLENCODED
    auto fFillRequestBodyKeyValueTable = [](const QVector<UtilFRequest::HttpFormKeyValue> &bodyForm, QTableWidget * const table){
        for(const UtilFRequest::HttpFormKeyValue &currFormKeyValue : bodyForm){
            Util::TableWidget::addRow(table, QStringList() << currFormKeyValue.key << currFormKeyValue.value);
        }
    };

    switch(info.bodyType){
    case UtilFRequest::BodyType::RAW:
    {
        ui->cbBodyType->setCurrentText("raw");
        ui->pteRequestBody->setPlainText(info.body);
    }
        break;
    case UtilFRequest::BodyType::FORM_DATA:
    {
        ui->cbBodyType->setCurrentText("form-data");
        fFillRequestBodyKeyValueTable(info.bodyForm, ui->twRequestBodyKeyValue);
    }
        break;
    case UtilFRequest::BodyType::X_FORM_WWW_URLENCODED:
    {
        ui->cbBodyType->setCurrentText("x-form-www-urlencoded");
        fFillRequestBodyKeyValueTable(info.bodyForm, ui->twRequestBodyKeyValue);
    }
        break;
    default:
        break;
    }

    ui->pteRequestBody->setPlainText(info.body);

    for(const UtilFRequest::HttpHeader &currentHeader : info.headers){
        Util::TableWidget::addRow(ui->twRequestHeadersKeyValue, QStringList() << currentHeader.name << currentHeader.value);
    }

    ui->cbDownloadResponseAsFile->setChecked(info.bDownloadResponseAsFile);

    clearOlderResponse();

    this->ignoreAnyChangesToProject = false;
}

void MainWindow::clearOlderResponse(){
    this->lastReplyStatusError = 0;
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
    // will then call update title that will try to acess pointer to unexisting objects)
    this->uuidsInUse.clear();
    this->uuidsToCleanUp.clear();
    this->currentItem = nullptr;
    this->currentProjectItem = nullptr;
    ui->treeWidget->clear();
    ui->leMainUrl->clear();
    clearRequestAndResponse();
}

void MainWindow::on_cbResponseChooseHeader_currentIndexChanged(const QString &arg1)
{

    if(arg1 == "Content-type: application/json"){
        Util::TableWidget::addRow(ui->twRequestHeadersKeyValue, QStringList() << "Content-type" << "application/json");
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
    Util::TableWidget::addRow(ui->twRequestBodyKeyValue, QStringList() << "" << "");
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

    if(!ui->twRequestBodyKeyValue->item(row, column)->text().trimmed().isEmpty()){
        ui->twRequestBodyKeyValue->resizeColumnsToContents();
    }
}

void MainWindow::on_cbBodyType_currentIndexChanged(const QString &arg1)
{
    ui->pteRequestBody->hide();
    ui->twRequestBodyKeyValue->hide();
    ui->tbRequestBodyKeyValueAdd->hide();
    ui->tbRequestBodyKeyValueRemove->hide();

    if(arg1 == "raw"){
        ui->pteRequestBody->show();
    }
    else{
        ui->twRequestBodyKeyValue->show();
        ui->tbRequestBodyKeyValueAdd->show();
        ui->tbRequestBodyKeyValueRemove->show();
    }

    if(!this->ignoreAnyChangesToProject){
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
    bool toogle = UtilFRequest::requestTypeMayHaveBody(UtilFRequest::getRequestTypeByText(ui->cbRequestType->currentText()));

    setProjectHasChanged();

    ui->cbBodyType->setEnabled(toogle);
    ui->twRequestBodyKeyValue->setEnabled(toogle);
    ui->pteRequestBody->setEnabled(toogle);
    ui->tbRequestBodyKeyValueAdd->setEnabled(toogle);
    ui->tbRequestBodyKeyValueRemove->setEnabled(toogle);

    if(!this->ignoreAnyChangesToProject){
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

void MainWindow::on_pteRequestBody_textChanged()
{
    setProjectHasChanged();
}

void MainWindow::on_cbDownloadResponseAsFile_toggled(bool)
{
    setProjectHasChanged();
}

void MainWindow::setProjectHasChanged(){

    if(!this->ignoreAnyChangesToProject && !this->unsavedChangesExist){
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

    auto fRequestFormToString = [](const QVector<UtilFRequest::HttpFormKeyValue> &requestForm){

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
        if(ui->cbBodyType->currentText() == "raw"){
            textToClipboard = ui->pteRequestBody->toPlainText();
        }
        else if(ui->cbBodyType->currentText() == "form-data"){
            textToClipboard = fRequestFormToString(getRequestForm());
        }
        else if(ui->cbBodyType->currentText() == "x-form-www-urlencoded"){
            textToClipboard = fRequestFormToString(getRequestForm());
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

    UtilFRequest::RequestType currentRequestType = UtilFRequest::getRequestTypeByText(ui->cbRequestType->currentText());


    std::experimental::optional<ConfigFileFRequest::ProtocolHeader>& currentProtocolHeader = ConfigFileFRequest::getSettingsHeaderForRequestType(currentRequestType, this->currentSettings);

    if(currentProtocolHeader.has_value()){
        std::experimental::optional<QVector<UtilFRequest::HttpHeader>> *currentHeaders = nullptr;

        if(ui->cbBodyType->currentText() == "raw"){
            currentHeaders = &currentProtocolHeader.value().headers_Raw;
        }
        else if(ui->cbBodyType->currentText() == "form-data"){
            currentHeaders = &currentProtocolHeader.value().headers_Form_Data;
        }
        else if(ui->cbBodyType->currentText() == "x-form-www-urlencoded"){
            currentHeaders = &currentProtocolHeader.value().headers_X_form_www_urlencoded;
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
    ui->cbRequestType->setCurrentText(UtilFRequest::getRequestTypeText(requestType));
}

void MainWindow::setFormatRequestAndResponseBodies(){
    if(ui->actionFormat_Request_body->isChecked()){
        this->requestBodyHighligher.setDocument(ui->pteRequestBody->document());
    }
    else{
        this->requestBodyHighligher.setDocument(nullptr);
    }

    if(ui->actionFormat_Response_Body->isChecked()){
        this->responseBodyHighligher.setDocument(ui->pteResponseBody->document());
    }
    else{
        this->responseBodyHighligher.setDocument(nullptr);
    }
}

bool MainWindow::loadAndValidateFRequestProjectFile(const QString &filePath, pugi::xml_document &doc){
    pugi::xml_parse_result result = doc.load_file(QSTR_TO_CSTR(filePath));

    if(result.status!=pugi::status_ok){
        QString errorMessage = "An error ocurred while loading project file.\n" + QString(result.description());
        LOG_ERROR << errorMessage;
        Util::Dialogs::showError(errorMessage);
        return false;
    }

    if(QString(doc.root().first_child().name()) != "FRequestProject"){
        QString errorMessage = QString(doc.root().name()) + "The file opened is not a valid FRequestProject file. Load aborted.";
        LOG_ERROR << errorMessage;
        Util::Dialogs::showError(errorMessage);
        return false;
    }

    QString projFRequestVersion;

    try{
        projFRequestVersion = QString(doc.select_node("/FRequestProject/@frequestVersion").attribute().value());
    }
    catch (const pugi::xpath_exception& e)
    {
        QString errorMessage = "Couldn't find the frequestVersion of the current project. Load aborted.\n" + QString(e.what());
        LOG_ERROR << errorMessage;
        Util::Dialogs::showError(errorMessage);
        return false;
    }

    if(!projFRequestVersion.startsWith(GlobalVars::LastCompatibleVersion)){
        QString errorMessage = "The project that you are trying to load seems it is not compatible with your FRequest Version. Please update FRequest and try again.";
        LOG_ERROR << errorMessage;
        Util::Dialogs::showError(errorMessage);
        return false;
    }

    return true;
}

pugi::xml_attribute MainWindow::createOrGetPugiXmlAttribute(pugi::xml_node &mainNode, const char *name){
    // if attributes doesnt exists create it
    if(mainNode.attribute(name).empty())
    {
        return mainNode.append_attribute(name);
    }
    else
    { // if it already exists return it
        return mainNode.attribute(name);
    }
}
