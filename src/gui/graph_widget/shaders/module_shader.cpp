#include "gui/graph_widget/shaders/module_shader.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/gui_globals.h"
#include "gui/module_model/module_item.h"
#include "netlist/module.h"

bool module_shader::s_color_gates = true;    // SET VIA SETTING

module_shader::module_shader(const graph_context* const context) : graph_shader(context)
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

    for (u32 id : m_context->modules())
    {
        module_item* item = g_netlist_relay.get_module_item(id);
        assert(item);

        graphics_node::visuals v;
        v.main_color = item->color();
        m_shading.module_visuals.insert(id, v);
    }

    for (u32 id : m_context->gates())
    {
        std::shared_ptr<gate> g = g_netlist->get_gate_by_id(id);
        assert(g);

        std::shared_ptr<module> m = g->get_module();
        assert(m);

        if (m->get_id())
        {
            module_item* item = g_netlist_relay.get_module_item(m->get_id());
            assert(item);

            graphics_node::visuals v;
            v.main_color = item->color();
            m_shading.gate_visuals.insert(id, v);
        }
    }
}

void module_shader::reset()
{
    m_shading.module_visuals.clear();
    m_shading.gate_visuals.clear();
    m_shading.net_visuals.clear();
}
