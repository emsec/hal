#include "gui/graph_widget/shaders/module_shader.h"

#include "netlist/module.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/gui_globals.h"
namespace hal{
module_shader::module_shader(const graph_context* const context) : graph_shader(context),
    m_color_gates(true) // INITIALIZE WITH DEFAULT VALUE FROM SETTINGS
{
}

void module_shader::add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets)
{
    Q_UNUSED(modules)
    Q_UNUSED(gates)
    Q_UNUSED(nets)
}

void module_shader::remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets)
{
    Q_UNUSED(modules)
    Q_UNUSED(gates)
    Q_UNUSED(nets)
}

void module_shader::update()
{
    m_shading.module_visuals.clear();
    m_shading.gate_visuals.clear();
    m_shading.net_visuals.clear();

    for (const u32& id : m_context->modules())
    {
        graphics_node::visuals v;
        v.main_color = g_netlist_relay.get_module_color(id);
        v.visible = true;
        m_shading.module_visuals.insert(id, v);
    }

    if (m_color_gates)
    {
        for (const u32& id : m_context->gates())
        {
            std::shared_ptr<Gate> g = g_netlist->get_gate_by_id(id);
            assert(g);

            std::shared_ptr<Module> m = g->get_module();
            assert(m);

            graphics_node::visuals v;
            v.main_color = g_netlist_relay.get_module_color(m->get_id());
            v.visible = true;
            m_shading.gate_visuals.insert(id, v);
        }
    }

    // DEBUG CODE
    for (const u32& id : m_context->nets())
    {
        graphics_net::visuals v{true, QColor(200, 200, 200), Qt::SolidLine, true, QColor(100, 100, 100), Qt::SolidPattern};
        m_shading.net_visuals.insert(id, v);
    }
}
}
