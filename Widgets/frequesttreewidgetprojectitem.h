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

#ifndef FREQUESTTREEWIDGETPROJECTITEM_H
#define FREQUESTTREEWIDGETPROJECTITEM_H

#include "frequesttreewidgetrequestitem.h"

class FRequestTreeWidgetProjectItem : public FRequestTreeWidgetItem
{
public:
    FRequestTreeWidgetProjectItem(const QStringList &list, const QString &uuid);
	
public:
    static FRequestTreeWidgetProjectItem* fromQTreeWidgetItem(QTreeWidgetItem* widget);
	QString getUuid();
	FRequestTreeWidgetRequestItem * getChildRequestByUuid(const QString &requestUuid);
    virtual void addRequestItemChild(FRequestTreeWidgetRequestItem * const child);
public:
	QString projectName;
	QString projectMainUrl = "http://localhost/";
	// can't use optional because it is an abstract base class, using unique_ptr as nullptr as alternative
    std::shared_ptr<FRequestAuthentication> authData = nullptr;
private:
    QString uuid;
    QHash<QString, FRequestTreeWidgetRequestItem *> mapOfChilds_UuidToRequest;
};

#endif // FREQUESTTREEWIDGETPROJECTITEM_H
