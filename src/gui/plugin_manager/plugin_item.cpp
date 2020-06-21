#include "plugin_manager/plugin_item.h"

#include <QObject>

namespace hal
{
    namespace __plugin_item_impl
    {
        QVector<QPair<QString, PluginItem::column_t>> column_desc = {{{QObject::tr("Name"), PluginItem::column_t::name}, {QObject::tr("Path"), PluginItem::column_t::path}}};
    }

    PluginItem::PluginItem() : name(""), path("")
    {
    }

    PluginItem::PluginItem(QString plugin_name, QString plugin_path) : name(plugin_name), path(plugin_path)
    {
    }

    bool PluginItem::is_valid()
    {
        if (name.compare("") != 0 && path.compare("") != 0)
            return true;
        else
            return false;
    }

    QVector<QPair<QString, PluginItem::column_t>> PluginItem::get_column_description()
    {
        return __plugin_item_impl::column_desc;
    }
}
