#include "gui/graph_widget/contexts/dynamic_context.h"

#include "gui/gui_globals.h"

dynamic_context::dynamic_context(const QString& name) : graph_context(context_type::dynamic, g_graph_context_manager.get_default_layouter(this), g_graph_context_manager.get_default_shader(this)),
    m_name(name)
{

}

QString dynamic_context::name() const
{
    return m_name;
}
