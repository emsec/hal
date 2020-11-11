#include "gui/plugin_management/plugin_relay.h"

#include "hal_core/plugin_system/plugin_manager.h"

namespace hal
{
    PluginRelay::PluginRelay(QObject* parent) : QObject(parent)
    {
        mCallbackId = plugin_manager::add_model_changed_callback(std::bind(&PluginRelay::pluginManagerCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    PluginRelay::~PluginRelay()
    {
        plugin_manager::remove_model_changed_callback(mCallbackId);
    }

    void PluginRelay::pluginManagerCallback(bool is_load, const std::string& plugin_name, const std::string& plugin_path)
    {
        QString name = QString::fromStdString(plugin_name);
        QString path = QString::fromStdString(plugin_path);

        if (is_load)
            Q_EMIT pluginLoaded(name, path);
        else
            Q_EMIT pluginUnloaded(name, path);
    }
}
