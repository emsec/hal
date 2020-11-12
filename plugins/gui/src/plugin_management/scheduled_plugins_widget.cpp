#include "gui/plugin_management/scheduled_plugins_widget.h"

#include "gui/plugin_management/scheduled_plugin_item_area.h"

namespace hal
{
    ScheduledPluginsWidget::ScheduledPluginsWidget(QWidget* parent) : QScrollArea(parent), mArea(new ScheduledPluginItemArea())
    {
        setFrameStyle(QFrame::NoFrame);
        setWidgetResizable(true);
        setWidget(mArea);
    }

    ScheduledPluginItemArea* ScheduledPluginsWidget::area()
    {
        return mArea;
    }

    void ScheduledPluginsWidget::appendPlugin(const QString& name)
    {
        mArea->insertPlugin(name);
        show();
    }
}
