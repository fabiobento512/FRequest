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

#ifndef FREQUESTTREEWIDGETITEM_H
#define FREQUESTTREEWIDGETITEM_H

#include <QTreeWidgetItem>
#include <cpp17optional/optional.hpp>

#include "utilfrequest.h"

class FRequestTreeWidgetItem : public QTreeWidgetItem
{
public:
    FRequestTreeWidgetItem(const QStringList &list, const bool isProjectItem = false);
    static FRequestTreeWidgetItem* fromQTreeWidgetItem(QTreeWidgetItem* widget);
    bool hasUuid();
public:
    struct RequestInfo{
        bool bOverridesMainUrl = false;
        QString overrideMainUrl;
        QString name;
        QString path;
        UtilFRequest::RequestType requestType = UtilFRequest::RequestType::GET_OPTION;
        UtilFRequest::BodyType bodyType = UtilFRequest::BodyType::RAW;
        QString body;
        QVector<UtilFRequest::HttpFormKeyValue> bodyForm;
        QVector<UtilFRequest::HttpHeader> headers;
        bool bDownloadResponseAsFile = false;
        QString uuid;
        unsigned long long int order = 0;
    };
public:
    RequestInfo itemContent;
    const bool isProjectItem;
};

#endif // FREQUESTTREEWIDGETITEM_H
