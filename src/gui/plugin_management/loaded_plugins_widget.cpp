#include "plugin_management/loaded_plugins_widget.h"

#include "core/plugin_manager.h"
#include "gui_globals.h"
#include "plugin_management/loaded_plugin_item.h"

#include <QFrame>
#include <QVBoxLayout>

loaded_plugins_widget::loaded_plugins_widget(QWidget* parent) : QScrollArea(parent), m_container(new QFrame()), m_layout(new QVBoxLayout()), m_spacer(new QFrame())
{
    connect(&g_plugin_relay, &plugin_relay::plugin_loaded, this, &loaded_plugins_widget::handle_plugin_loaded);
    connect(&g_plugin_relay, &plugin_relay::plugin_unloaded, this, &loaded_plugins_widget::handle_plugin_unloaded);

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

void loaded_plugins_widget::handle_plugin_loaded(const QString& name, const QString& path)
{
    Q_UNUSED(path)

    auto factory_ptr = plugin_manager::get_plugin_factory(name.toStdString());
    if (factory_ptr == nullptr)
    {
        return;
    }
    auto plugin_types = factory_ptr->get_plugin_types();
    if (plugin_types.find(interface_type::cli) == plugin_types.end())
    {
        return;
    }

    m_layout->addWidget(new loaded_plugin_item(name, this));
}

void loaded_plugins_widget::handle_plugin_unloaded(const QString& name, const QString& path)
{
    Q_UNUSED(path)

    for (QObject* child : m_layout->children())
    {
        loaded_plugin_item* item = static_cast<loaded_plugin_item*>(child);

        if (item->name() == name)
            item->deleteLater();
    }
}
