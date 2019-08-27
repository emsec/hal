#include "gui/graph_widget/contexts/dynamic_context.h"

#include "gui/gui_globals.h"

#include "netlist/gate.h"
#include "netlist/module.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

#include <QDebug>

dynamic_context::dynamic_context(const QString& name)
    : graph_context(context_type::dynamic, g_graph_context_manager.get_default_layouter(this), g_graph_context_manager.get_default_shader(this)), m_name(name)
{
}

QString dynamic_context::name() const
{
    return m_name;
}

void dynamic_context::apply_changes()
{
    graph_context::apply_changes();

    m_layouter->remove({}, {}, m_nets);
    m_shader->remove({}, {}, m_nets);
    m_nets.clear();
    for (const auto& id : m_gates)
    {
        auto g = g_netlist->get_gate_by_id(id);
        for (const auto& net : g->get_fan_in_nets())
        {
            m_nets.insert(net->get_id());
        }
        for (const auto& net : g->get_fan_out_nets())
        {
            m_nets.insert(net->get_id());
        }
    }
    for (const auto& id : m_modules)
    {
        auto m = g_netlist->get_module_by_id(id);
        for (const auto& net : m->get_input_nets())
        {
            m_nets.insert(net->get_id());
        }
        for (const auto& net : m->get_output_nets())
        {
            m_nets.insert(net->get_id());
        }
    }
    m_layouter->add({}, {}, m_nets);
    m_shader->add({}, {}, m_nets);
}
