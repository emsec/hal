#include "plugin_manager/plugin_item.h"

#include <QObject>

namespace hal
{
    namespace __plugin_item_impl
    {
        QVector<QPair<QString, plugin_item::column_t>> column_desc = {{{QObject::tr("Name"), plugin_item::column_t::name}, {QObject::tr("Path"), plugin_item::column_t::path}}};
    }

    plugin_item::plugin_item() : name(""), path("")
    {
    }

    plugin_item::plugin_item(QString plugin_name, QString plugin_path) : name(plugin_name), path(plugin_path)
    {
    }

    bool plugin_item::is_valid()
    {
        if (name.compare("") != 0 && path.compare("") != 0)
            return true;
        else
            return false;
    }

    QVector<QPair<QString, plugin_item::column_t>> plugin_item::get_column_description()
    {
        return __plugin_item_impl::column_desc;
    }
}
