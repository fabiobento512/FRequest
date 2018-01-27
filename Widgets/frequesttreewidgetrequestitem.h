/*
 *
Copyright (C) 2017-2018  Fábio Bento (random-guy)

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

#ifndef FREQUESTTREEWIDGETREQUESTITEM_H
#define FREQUESTTREEWIDGETREQUESTITEM_H

#include "frequesttreewidgetitem.h"

class FRequestTreeWidgetRequestItem : public FRequestTreeWidgetItem
{
public:
    FRequestTreeWidgetRequestItem(const QStringList &list, const QString &uuid);
    static FRequestTreeWidgetRequestItem* fromQTreeWidgetItem(QTreeWidgetItem* widget);
public:
    UtilFRequest::RequestInfo itemContent;
};

Q_DECLARE_METATYPE(FRequestTreeWidgetRequestItem*) // necessary for qvariant_cast

#endif // FREQUESTTREEWIDGETREQUESTITEM_H
