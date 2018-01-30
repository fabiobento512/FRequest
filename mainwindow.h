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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkProxyFactory>
#include <QTimer>
#include <QTreeWidgetItem>
#include <QStringBuilder>
#include <QUrlQuery>
#include <QClipboard>
#include <QUuid>
#include <QProgressBar>
#include <QToolButton>
#include <QLabel>
#include <QPainter>
#include <QMap>

#include <jsonhighlighter/highlighter.h>
#include <BasicXMLSyntaxHighlighter/BasicXMLSyntaxHighlighter.h>
#include <pugixml/pugixml.hpp>
#include <ConditionalSemaphore/conditionalsemaphore.h>

#include "about.h"
#include "preferences.h"
#include "projectproperties.h"
#include "utilfrequest.h"
#include "Widgets/frequesttreewidgetprojectitem.h"

#include "XmlParsers/projectfilefrequest.h"
#include "XmlParsers/configfilefrequest.h"

#include "HttpRequests/posthttprequest.h"
#include "HttpRequests/puthttprequest.h"
#include "HttpRequests/gethttprequest.h"
#include "HttpRequests/deletehttprequest.h"
#include "HttpRequests/patchhttprequest.h"
#include "HttpRequests/headhttprequest.h"
#include "HttpRequests/tracehttprequest.h"
#include "HttpRequests/optionshttprequest.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void applicationHasLoaded();
	
	void saveProjectProperties();
	
	void treeWidgetDeleteKeyPressed();

    void on_pbSendRequest_clicked();

    void on_lePath_textChanged(const QString &arg1);

    void replyFinished(QNetworkReply *reply);

    void on_treeWidget_customContextMenuRequested(const QPoint &pos);

    void on_treeWidget_itemChanged(QTreeWidgetItem *item, int column);

    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_actionSave_Project_triggered();

    void on_actionNew_Project_triggered();

    void on_actionSave_Project_As_triggered();

    void on_actionLoad_Project_triggered();

    void on_actionExit_triggered();

    void on_actionAbout_triggered();

    void on_actionProject1_triggered();

    void on_actionProject2_triggered();

    void on_actionProject3_triggered();

    void on_actionProject4_triggered();

    void on_actionProject5_triggered();

    void on_actionProject6_triggered();

    void on_cbResponseChooseHeader_currentIndexChanged(const QString &arg1);

    void on_tbRequestBodyKeyValueAdd_clicked();

    void on_twRequestBodyKeyValue_cellChanged(int row, int column);

    void on_cbBodyType_currentIndexChanged(const QString &arg1);

    void on_tbRequestBodyKeyValueRemove_clicked();

    void on_tbRequestHeadersKeyValueAdd_clicked();

    void on_tbRequestHeadersKeyValueRemove_clicked();

    void on_twRequestHeadersKeyValue_cellChanged(int row, int column);

    void on_cbRequestOverrideMainUrl_toggled(bool checked);

    void on_leRequestOverrideMainUrl_textChanged(const QString &arg1);

    void on_pteRequestBody_textChanged();

    void on_cbDownloadResponseAsFile_toggled(bool checked);

    void on_tbCopyToClipboardRequest_clicked();

    void on_tbCopyToClipboardResponse_clicked();

    void on_actionPreferences_triggered();

    void saveCurrentSettings();

    void on_cbRequestType_currentIndexChanged(const QString &arg1);

    void on_actionFormat_Response_Body_triggered();

    void on_actionFormat_Request_body_triggered();

    void on_tbRequestBodyFile_clicked();

    void tbAbortRequest_clicked();

    void on_actionShow_Request_Types_Icons_triggered(bool checked);

    void on_leRequestsFilter_textChanged(const QString &arg1);
	
	void on_actionProject_Properties_triggered();

signals:
    void signalAppIsLoaded();
	void signalRequestFinishedAndProcessed();

private:
    void showEvent(QShowEvent *e);
    void checkForQNetworkAccessManagerTimeout(QNetworkReply *reply);
    QNetworkReply* processHttpRequest(
            const UtilFRequest::RequestType requestType,
            const QString &fullPath,
            const QString &bodyType,
            const QString &requestBody,
            const QVector<UtilFRequest::HttpHeader>& requestHeaders
    );
    QString getDownloadFileName(const QNetworkReply * const reply);
    void updateWindowTitle();
    void setNewProject();
    void saveProjectState(const QString &filePath);
    void loadProjectState(const QString &filePath);
    void updateTreeWidgetItemContent(FRequestTreeWidgetRequestItem * const requestItem);
    void reloadRequest(FRequestTreeWidgetRequestItem * const item);
    FRequestTreeWidgetProjectItem *addProjectItem(const QString &projectName, const QString &projectUuid);
    FRequestTreeWidgetRequestItem *addRequestItem(const QString &requestName, const QString &projectUuid, FRequestTreeWidgetProjectItem * const currentProject);
    void closeEvent(QCloseEvent *event);
    void loadRecentProjects();
    void addNewRecentProject(const QString &filePath);
    void reloadRecentProjectsMenu();
    void saveRecentProjects();
    QVector<UtilFRequest::HttpHeader> getRequestHeaders();
    QVector<UtilFRequest::HttpFormKeyValueType> getRequestForm();
    void buildFullPath();
    void setProjectHasChanged();
    QMessageBox::StandardButton askToSaveCurrentProject();
    void clearOlderResponse();
    void clearRequestAndResponse();
    void clearEverything();
    void addDefaultHeaders();
    void setRequestType(UtilFRequest::RequestType requestType);
    void formatRequestBody(const UtilFRequest::SerializationFormatType serializationType);
	void formatResponseBody(const UtilFRequest::SerializationFormatType serializationType);
    bool loadAndValidateFRequestProjectFile(const QString &filePath, pugi::xml_document &doc);
    void setIconForRequest(FRequestTreeWidgetRequestItem * const item);
    void setAllRequestIcons(bool showIcon);
	ProjectFileFRequest::ProjectData fetchCurrentProjectData();
	bool formKeyValueInBodyIsValid();
	bool formKeyValueInBodyHasFiles();
	void removeAllFilesRowsFromFormKeyValueInBody();
	UtilFRequest::SerializationFormatType getRequestCurrentSerializationFormatType();
	UtilFRequest::SerializationFormatType getResponseCurrentSerializationFormatType();
	QString getNewUuid();
    QString getFullPathFromMainUrlAndPath(const QString & mainUrl, const QString & path);
	void applyRequestAuthentication();
	void openProjectProperties();
	void removeRequest(FRequestTreeWidgetRequestItem * const itemToDelete);

public:
    static constexpr int recentProjectsMaxSize=6;
	
private:
    Ui::MainWindow *ui;
    QDateTime lastStartTime;
    FRequestTreeWidgetProjectItem *currentProjectItem = nullptr;
    FRequestTreeWidgetRequestItem *currentItem = nullptr;
    QSet<QString> uuidsInUse;
    QVector<QString> uuidsToCleanUp; // this vector stores the uuids of deleted items in the application, its used to clean them in the project file
    bool applicationIsFullyLoaded = false;
    bool unsavedChangesExist = false;
    // This conditional semaphore allow us to tell to the interface to ignore changes (not mark project as unsaved) within some time interval
	Cosemaphore::ConditionalSemaphore ignoreAnyChangesToProject;
    QString lastProjectFilePath;
    QList<QString> recentProjectsList;
    QString lastResponseFileName;
    int lastReplyStatusError = 0;
    ConfigFileFRequest configFileManager = ConfigFileFRequest(Util::FileSystem::getAppPath() + "/" + GlobalVars::AppConfigFileName);
    ConfigFileFRequest::Settings currentSettings;
    const QSize auxMinimumSize = QSize(0,0);
    const QSize auxMaximumSize = QSize(16777215,16777215);
    QProgressBar pbRequestProgress;
    QToolButton tbAbortRequest;
    QLabel lbRequestInfo;
    QLabel lbProjectInfo;
    std::experimental::optional<QNetworkReply*> currentReply;
    QMap<UtilFRequest::RequestType, QIcon> generatedIconCache;
    QNetworkAccessManager networkAccessManager;
	
	// Requests Highlighters
	Highlighter jsonRequestBodyHighligher;
    Highlighter jsonResponseBodyHighligher;
	BasicXMLSyntaxHighlighter xmlRequestBodyHighligher;
	BasicXMLSyntaxHighlighter xmlResponseBodyHighligher;
    bool currentProjectAuthenticationWasMade = false;
	bool authenticationIsRunning = false;

};

#endif // MAINWINDOW_H
