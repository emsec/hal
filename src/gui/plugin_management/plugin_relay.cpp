#include "plugin_management/plugin_relay.h"

#include "core/plugin_manager.h"

namespace hal
{
    plugin_relay::plugin_relay(QObject* parent) : QObject(parent)
    {
        PluginManager::add_model_changed_callback(std::bind(&plugin_relay::plugin_manager_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    void plugin_relay::plugin_manager_callback(bool is_load, const std::string& plugin_name, const std::string& plugin_path)
    {
        QString name = QString::fromStdString(plugin_name);
        QString path = QString::fromStdString(plugin_path);

        if (is_load)
            Q_EMIT plugin_loaded(name, path);
        else
            Q_EMIT plugin_unloaded(name, path);
    }
}
