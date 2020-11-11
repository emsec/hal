#include "gui/plugin_manager/plugin_item.h"

#include <QObject>

namespace hal
{
    namespace __plugin_item_impl
    {
        QVector<QPair<QString, PluginItem::ColumnType>> column_desc = {{{QObject::tr("Name"), PluginItem::ColumnType::Name},
                                                                        {QObject::tr("Path"), PluginItem::ColumnType::Path}}};
    }

    PluginItem::PluginItem()
    {;}

    PluginItem::PluginItem(const QString& plugin_name, const QString& plugin_path)
        : mName(plugin_name), mPath(plugin_path)
    {;}

    bool PluginItem::isValid()
    {
        if (!mName.isEmpty() && !mPath.isEmpty())
            return true;
        else
            return false;
    }

    QVector<QPair<QString, PluginItem::ColumnType>> PluginItem::getColumnDescription()
    {
        return __plugin_item_impl::column_desc;
    }
}
