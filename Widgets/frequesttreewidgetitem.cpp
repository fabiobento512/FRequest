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
