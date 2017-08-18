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
#include <QJsonDocument>
#include <QUrlQuery>
#include <QClipboard>
#include <QUuid>

#include <jsonhighlighter/highlighter.h>
#include <pugixml/pugixml.hpp>

#include "about.h"
#include "preferences.h"
#include "utilfrequest.h"
#include "utilglobalvars.h"
#include "configfilefrequest.h"
#include "Widgets/frequesttreewidgetitem.h"

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

    void on_pbSendRequest_clicked();

    void on_leMainUrl_textChanged(const QString &arg1);

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

signals:
    void signalAppIsLoaded();

private:
    void showEvent(QShowEvent *e);
    void checkForQNetworkAccessManagerTimeout(QNetworkReply *reply);
    QNetworkReply* processHttpRequest(QNetworkAccessManager * const manager, UtilFRequest::RequestType requestType);
    QString getDownloadFileName(const QNetworkReply * const reply);
    void updateWindowTitle();
    void setNewProject();
    void saveProjectState(const QString &filePath);
    void loadProjectState(const QString &filePath);
    void updateTreeWidgetItemContent(FRequestTreeWidgetItem * const requestItem);
    void reloadRequest(FRequestTreeWidgetItem * const item);
    FRequestTreeWidgetItem *addProjectItem(const QString &projectName);
    FRequestTreeWidgetItem *addRequestItem(const QString &requestName, FRequestTreeWidgetItem * const currentProject);
    void closeEvent(QCloseEvent *event);
    void loadRecentProjects();
    void addNewRecentProject(const QString &filePath);
    void reloadRecentProjectsMenu();
    void saveRecentProjects();
    QVector<UtilFRequest::HttpHeader> getRequestHeaders();
    QVector<UtilFRequest::HttpFormKeyValue> getRequestForm();
    void buildFullPath();
    void setProjectHasChanged();
    QMessageBox::StandardButton askToSaveCurrentProject();
    void clearOlderResponse();
    void clearRequestAndResponse();
    void clearEverything();
    void addDefaultHeaders();
    void setRequestType(UtilFRequest::RequestType requestType);
    void setFormatRequestAndResponseBodies();
    bool loadAndValidateFRequestProjectFile(const QString &filePath, pugi::xml_document &doc);
    pugi::xml_attribute createOrGetPugiXmlAttribute(pugi::xml_node &mainNode, const char *name);

public:
    static constexpr int recentProjectsMaxSize=6;

private:
    Ui::MainWindow *ui;
    QDateTime lastStartTime;
    FRequestTreeWidgetItem *currentProjectItem = nullptr;
    FRequestTreeWidgetItem *currentItem = nullptr;
    QSet<QString> uuidsInUse;
    QVector<QString> uuidsToCleanUp; // this vector stores the uuids of deleted items in the application, its used to clean them in the project file
    bool applicationIsFullyLoaded = false;
    bool unsavedChangesExist = false;
    // aux boolean to indicate that the current changes are to ignore (used when changing items in the treeview or when program starts up)
    bool ignoreAnyChangesToProject = true;
    QString lastProjectFilePath;
    QList<QString> recentProjectsList;
    QString lastResponseFileName;
    int lastReplyStatusError = 0;
    ConfigFileFRequest configFileManager = ConfigFileFRequest(Util::FileSystem::getAppPath() + "/" + GlobalVars::AppConfigFileName);
    ConfigFileFRequest::Settings currentSettings;
    const QSize auxMinimumSize = QSize(0,0);
    const QSize auxMaximumSize = QSize(16777215,16777215);
    Highlighter requestBodyHighligher;
    Highlighter responseBodyHighligher;
};

#endif // MAINWINDOW_H
