#include "gui/plugin_management/plugin_relay.h"

#include "core/plugin_manager.h"

namespace hal
{
    PluginRelay::PluginRelay(QObject* parent) : QObject(parent)
    {
        m_callback_id = plugin_manager::add_model_changed_callback(std::bind(&PluginRelay::plugin_manager_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    PluginRelay::~PluginRelay()
    {
        plugin_manager::remove_model_changed_callback(m_callback_id);
    }

    void PluginRelay::plugin_manager_callback(bool is_load, const std::string& plugin_name, const std::string& plugin_path)
    {
        QString name = QString::fromStdString(plugin_name);
        QString path = QString::fromStdString(plugin_path);

        if (is_load)
            Q_EMIT plugin_loaded(name, path);
        else
            Q_EMIT plugin_unloaded(name, path);
    }
}
