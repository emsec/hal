#include "gui/graph_widget/contexts/dynamic_context.h"

#include "gui/gui_globals.h"

dynamic_context::dynamic_context(const QString& name, const u32 scope) : graph_context(g_graph_context_manager.get_default_layouter(this),
                                                                                       g_graph_context_manager.get_default_shader(this)),
    m_name(name),
    m_scope(scope)
{

}

QString dynamic_context::name() const
{
    return m_name;
}

u32 dynamic_context::scope() const
{
    return m_scope;
}
