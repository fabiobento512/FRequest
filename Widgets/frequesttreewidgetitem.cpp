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

#include "frequesttreewidgetitem.h"

FRequestTreeWidgetItem::FRequestTreeWidgetItem(const QStringList &list, const bool isProjectItem)
    :QTreeWidgetItem(list), isProjectItem(isProjectItem)
{
}

FRequestTreeWidgetItem* FRequestTreeWidgetItem::fromQTreeWidgetItem(QTreeWidgetItem* widget){
    FRequestTreeWidgetItem* result = dynamic_cast<FRequestTreeWidgetItem*>(widget);

    if(result == nullptr){
        QString errorString = "Fatal error, failed to convert QTreeWidgetItem* to FRequestTreeWidgetItem*. Widget Text: " +
        widget->text(0) + "Widget Position: " + QString::number(widget->parent()->indexOfChild(widget));

        LOG_FATAL << errorString;

        Util::Dialogs::showError(errorString + "\n Application can't proceed.");

        exit(-1);
    }

    return result;
}

bool FRequestTreeWidgetItem::hasUuid(){
    return !this->itemContent.uuid.isEmpty();
}
