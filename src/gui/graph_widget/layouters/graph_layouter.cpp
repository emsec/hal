#include "graph_widget/layouters/graph_layouter.h"

#include "gui_globals.h"

graph_layouter::graph_layouter(QObject* parent) : QObject(parent)
{
    connect(&g_graph_relay, &graph_relay::netlist_event, this, &graph_layouter::handle_netlist_event);
    connect(&g_graph_relay, &graph_relay::gate_event, this, &graph_layouter::handle_gate_event);
    connect(&g_graph_relay, &graph_relay::net_event, this, &graph_layouter::handle_net_event);
    connect(&g_graph_relay, &graph_relay::module_event, this, &graph_layouter::handle_module_event);
}

graph_layouter::~graph_layouter()
{
}

const QString& graph_layouter::get_name() const
{
    return m_name;
}

const QString& graph_layouter::get_description() const
{
    return m_description;
}
