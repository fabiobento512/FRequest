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

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>
#include <QSettings>
#include <QMessageBox>
#include <QShowEvent>

#include "util.h"
#include "XmlParsers/configfilefrequest.h"

namespace Ui {
class Preferences;
}

class Preferences : public QDialog
{
    Q_OBJECT
    
public:
    Preferences(QWidget *parent, ConfigFileFRequest::Settings &currentSettings);
    ~Preferences();

private slots:

	void accept ();

    void on_buttonBox_rejected();

    void on_cbUseDefaultHeaders_toggled(bool checked);

    void on_cbRequestType_currentIndexChanged(const QString &arg1);

    void on_tbRequestBodyKeyValueAdd_clicked();

    void on_tbRequestBodyKeyValueRemove_clicked();

    void on_cbRequestBodyType_currentIndexChanged(const QString &arg1);
    void preferencesHaveLoaded();

    void on_cbProxyUseProxy_toggled(bool checked);

    void on_cbProxyType_currentIndexChanged(const QString &arg1);

    void on_tbConfigProjDataRemove_clicked();

signals:
    void signalPreferencesAreLoaded();
    void saveSettings();

private:
    Ui::Preferences *ui;
    ConfigFileFRequest::Settings &currentSettings;
    QString previousRequestType = "GET"; // by default
    QString previousRequestBodyType = "raw";
    bool preferencesAreFullyLoaded = false;
    QVector<QString> configProjAuthsToDelete;

private:
    void showEvent(QShowEvent *e);
    void loadExistingSettings();
    std::experimental::optional<QVector<UtilFRequest::HttpHeader> > getRequestHeaders();
    void loadCurrentDefaultHeaders();
    void updateCurrentDefaultHeaders();
    void fillConfigProjAuthDataTable();
};

#endif // PREFERENCES_H
