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

#ifndef PROJECTPROPERTIES_H
#define PROJECTPROPERTIES_H

#include <QDialog>
#include <QCryptographicHash>

#include "customtreewidget.h"
#include "Widgets/frequesttreewidgetprojectitem.h"
#include "Authentications/basicauthentication.h"
#include "Authentications/requestauthentication.h"

namespace Ui {
class ProjectProperties;
}

class ProjectProperties : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectProperties(QWidget *parent, FRequestTreeWidgetProjectItem * const projectItem);
    ~ProjectProperties();

signals:
	void signalSaveProjectProperties();
	
private slots:
    void on_cbRequestType_currentIndexChanged(const QString &arg1);
    void accept ();

    void on_cbUseAuthentication_toggled(bool checked);

private:
    void fillInterface();
	void fillAuthenticationData(FRequestAuthentication &auth);
	QString getComboBoxNameForRequest(const FRequestTreeWidgetRequestItem* const currentRequest);
	
private:
    Ui::ProjectProperties *ui;
	FRequestTreeWidgetProjectItem* const projectItem; // not const because we can update its name in this class
	QString currentPasswordSalt;
};

#endif // PROJECTPROPERTIES_H
