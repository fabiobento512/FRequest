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
