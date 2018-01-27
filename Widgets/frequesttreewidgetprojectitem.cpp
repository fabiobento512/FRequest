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

#include "frequesttreewidgetprojectitem.h"

FRequestTreeWidgetProjectItem::FRequestTreeWidgetProjectItem(const QStringList &list, const QString &uuid)
    :FRequestTreeWidgetItem(list, true), uuid(uuid)
{
}

FRequestTreeWidgetProjectItem* FRequestTreeWidgetProjectItem::fromQTreeWidgetItem(QTreeWidgetItem* widget){
    FRequestTreeWidgetProjectItem* result = dynamic_cast<FRequestTreeWidgetProjectItem*>(widget);

    if(result == nullptr){
        QString errorString = "Fatal error, failed to convert QTreeWidgetItem* to FRequestTreeWidgetProjectItem*. Widget Text: " +
        widget->text(0) + "Widget Position: " + QString::number(widget->parent()->indexOfChild(widget));

        LOG_FATAL << errorString;

        Util::Dialogs::showError(errorString + "\n Application can't proceed.");

        exit(-1);
    }

    return result;
}

QString FRequestTreeWidgetProjectItem::getUuid(){
	return this->uuid;
}

FRequestTreeWidgetRequestItem * FRequestTreeWidgetProjectItem::getChildRequestByUuid(const QString &requestUuid){
    if(!this->mapOfChilds_UuidToRequest.contains(requestUuid)){
        QString errorString = "Fatal error, failed to get request item for the given uuid '" + requestUuid + "'";

        LOG_FATAL << errorString;

        Util::Dialogs::showError(errorString + "\n Application can't proceed.");

        exit(-1);
    }

    return this->mapOfChilds_UuidToRequest[requestUuid];
}

void FRequestTreeWidgetProjectItem::addChild(QTreeWidgetItem *child){
    FRequestTreeWidgetRequestItem* const currentRequest = FRequestTreeWidgetRequestItem::fromQTreeWidgetItem(child);
	
	// Add uuid to our child cache
    this->mapOfChilds_UuidToRequest.insert(currentRequest->itemContent.uuid, currentRequest);
	
	FRequestTreeWidgetItem::addChild(child);
}
