#include "plugin_management/scheduled_plugins_widget.h"

#include "plugin_management/scheduled_plugin_item_area.h"
namespace hal{
scheduled_plugins_widget::scheduled_plugins_widget(QWidget* parent) : QScrollArea(parent), m_area(new scheduled_plugin_item_area())
{
    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);
    setWidget(m_area);
}

scheduled_plugin_item_area* scheduled_plugins_widget::area()
{
    return m_area;
}

void scheduled_plugins_widget::append_plugin(const QString& name)
{
    m_area->insert_plugin(name);
    show();
}
}
