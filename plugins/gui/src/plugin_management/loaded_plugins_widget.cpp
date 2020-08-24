#include "gui/plugin_management/loaded_plugins_widget.h"

#include "core/plugin_interface_cli.h"
#include "core/plugin_manager.h"
#include "gui/gui_globals.h"
#include "gui/plugin_management/loaded_plugin_item.h"

#include <QFrame>
#include <QVBoxLayout>

namespace hal
{
    LoadedPluginsWidget::LoadedPluginsWidget(QWidget* parent) : QScrollArea(parent), m_container(new QFrame()), m_layout(new QVBoxLayout()), m_spacer(new QFrame())
    {
        connect(g_plugin_relay, &PluginRelay::plugin_loaded, this, &LoadedPluginsWidget::handle_plugin_loaded);
        connect(g_plugin_relay, &PluginRelay::plugin_unloaded, this, &LoadedPluginsWidget::handle_plugin_unloaded);

        m_container->setObjectName("container");

        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);
        m_layout->setAlignment(Qt::AlignTop);

        m_spacer->setObjectName("spacer");

        m_container->setLayout(m_layout);
        m_layout->addWidget(m_spacer);

        setFrameStyle(QFrame::NoFrame);
        setWidgetResizable(true);
        setWidget(m_container);
    }

    void LoadedPluginsWidget::handle_plugin_loaded(const QString& name, const QString& path)
    {
        Q_UNUSED(path)

        if (plugin_manager::get_plugin_instance<CLIPluginInterface>(name.toStdString(), false) == nullptr)
        {
            return;
        }

        m_layout->addWidget(new LoadedPluginItem(name, this));
    }

    void LoadedPluginsWidget::handle_plugin_unloaded(const QString& name, const QString& path)
    {
        Q_UNUSED(path)

        for (QObject* child : m_layout->children())
        {
            LoadedPluginItem* item = static_cast<LoadedPluginItem*>(child);

            if (item->name() == name)
                item->deleteLater();
        }
    }
}
