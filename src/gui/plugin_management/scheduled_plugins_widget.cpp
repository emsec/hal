#include "plugin_management/scheduled_plugins_widget.h"

#include "plugin_management/scheduled_plugin_item_area.h"

namespace hal
{
    ScheduledPluginsWidget::ScheduledPluginsWidget(QWidget* parent) : QScrollArea(parent), m_area(new ScheduledPluginItemArea())
    {
        setFrameStyle(QFrame::NoFrame);
        setWidgetResizable(true);
        setWidget(m_area);
    }

    ScheduledPluginItemArea* ScheduledPluginsWidget::area()
    {
        return m_area;
    }

    void ScheduledPluginsWidget::append_plugin(const QString& name)
    {
        m_area->insert_plugin(name);
        show();
    }
}
