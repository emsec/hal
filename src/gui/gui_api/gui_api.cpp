#include "gui_api/gui_api.h"

#include "gui_globals.h"

#include <algorithm>
#include <QSet>

gui_api::gui_api()
{
    g_selection_relay.register_sender(this, "GUI API");
}

gui_api::~gui_api()
{
    g_selection_relay.remove_sender(this);
}

std::vector<u32> gui_api::get_selected_gate_ids() const
{
    return std::vector<u32>(g_selection_relay.m_selected_gates.begin(), g_selection_relay.m_selected_gates.end());
}

std::vector<u32> gui_api::get_selected_net_ids() const
{
    return std::vector<u32>(g_selection_relay.m_selected_nets.begin(), g_selection_relay.m_selected_nets.end());
}

std::vector<u32> gui_api::get_selected_module_ids() const
{
    return std::vector<u32>(g_selection_relay.m_selected_modules.begin(), g_selection_relay.m_selected_modules.end());
}

std::tuple<std::vector<u32>, std::vector<u32>, std::vector<u32>> gui_api::get_selected_item_ids() const
{
    return std::make_tuple(get_selected_gate_ids(), get_selected_net_ids(), get_selected_module_ids());
}

std::vector<std::shared_ptr<gate>> gui_api::get_selected_gates() const
{
    std::vector<std::shared_ptr<gate>> gates(g_selection_relay.m_selected_gates.size());
    std::transform(g_selection_relay.m_selected_gates.begin(), g_selection_relay.m_selected_gates.end(), gates.begin(), [](u32 id){return g_netlist->get_gate_by_id(id);});

    return gates;
}

std::vector<std::shared_ptr<net>> gui_api::get_selected_nets() const
{   
    std::vector<std::shared_ptr<net>> nets(g_selection_relay.m_selected_nets.size());
    std::transform(g_selection_relay.m_selected_nets.begin(), g_selection_relay.m_selected_nets.end(), nets.begin(), [](u32 id){return g_netlist->get_net_by_id(id);});

    return nets;
}

std::vector<std::shared_ptr<module>> gui_api::get_selected_modules() const
{
    std::vector<std::shared_ptr<module>> modules(g_selection_relay.m_selected_modules.size());
    std::transform(g_selection_relay.m_selected_modules.begin(), g_selection_relay.m_selected_modules.end(), modules.begin(), [](u32 id){return g_netlist->get_module_by_id(id);});

    return modules;
}

std::tuple<std::vector<std::shared_ptr<gate>>, std::vector<std::shared_ptr<net>>, std::vector<std::shared_ptr<module>>> gui_api::get_selected_items() const
{
    return std::make_tuple(get_selected_gates(), get_selected_nets(), get_selected_modules());
}

/*
void gui_api::print_selected_gates() const
{
    u32 number_of_selected_gates = g_selection_relay.m_selected_gates.size();
    QString header = QString("Selected gates (%1):").arg(number_of_selected_gates);
    g_python_context->forward_stdout(header + "\n\n");
    g_python_context->forward_stdout("ID\tName\n");

    for(u32 gate_id : g_selection_relay.m_selected_gates)
    {
        auto gate = g_netlist->get_gate_by_id(gate_id);
        QString id = QString::number(gate->get_id());
        QString name = QString::fromStdString(gate->get_name());

        g_python_context->forward_stdout(id + "\t" + name + "\n");
    }
}

void gui_api::print_selected_nets() const
{
    u32 number_of_selected_nets = g_selection_relay.m_selected_nets.size();
    QString header = QString("Selected nets (%1):").arg(number_of_selected_nets);
    g_python_context->forward_stdout(header + "\n\n");
    g_python_context->forward_stdout("ID\tName\n");

    for(u32 net_id : g_selection_relay.m_selected_nets)
    {
        auto net = g_netlist->get_net_by_id(net_id);
        QString id = QString::number(net->get_id());
        QString name = QString::fromStdString(net->get_name());

        g_python_context->forward_stdout(id + "\t" + name + "\n");
    }
}

void gui_api::print_selected_modules() const
{
    u32 number_of_selected_modules = g_selection_relay.m_selected_modules.size();
    QString header = QString("Selected modules (%1):").arg(number_of_selected_modules);
    g_python_context->forward_stdout(header + "\n\n");
    g_python_context->forward_stdout("ID\tName\n");

    for(u32 module_id : g_selection_relay.m_selected_modules)
    {
        auto module = g_netlist->get_module_by_id(module_id);
        QString id = QString::number(module->get_id());
        QString name = QString::fromStdString(module->get_name());

        g_python_context->forward_stdout(id + "\t" + name + "\n");
    }
}

void gui_api::print_selected_items() const
{
    print_selected_gates();
    g_python_context->forward_stdout("\n");
    print_selected_nets();
    g_python_context->forward_stdout("\n");
    print_selected_modules();
}
*/

void gui_api::select_gate(const std::shared_ptr<gate>& gate, bool clear_current_selection)
{
    if(!g_netlist->is_gate_in_netlist(gate))
        return;

    if(clear_current_selection)
        g_selection_relay.clear();

    g_selection_relay.m_selected_gates.insert(gate->get_id());
    g_selection_relay.selection_changed(this);
}

void gui_api::select_gate(u32 gate_id, bool clear_current_selection)
{
    select_gate(g_netlist->get_gate_by_id(gate_id), clear_current_selection);
}

void gui_api::select_gate(const std::vector<std::shared_ptr<gate>>& gates, bool clear_current_selection)
{
    QSet<u32> gate_ids;

    for(auto gate : gates)
    {
        if(!g_netlist->is_gate_in_netlist(gate))
            return;

        gate_ids.insert(gate->get_id());    
    }

    if(clear_current_selection)
        g_selection_relay.clear();

    g_selection_relay.m_selected_gates.unite(gate_ids);
    g_selection_relay.selection_changed(this);
}

void gui_api::select_gate(const std::vector<u32>& gate_ids, bool clear_current_selection)
{
    std::vector<std::shared_ptr<gate>> gates(gate_ids.size());
    std::transform(gate_ids.begin(), gate_ids.end(), gates.begin(), [](u32 gate_id){return g_netlist->get_gate_by_id(gate_id);});
    select_gate(gates, clear_current_selection);
}

void gui_api::select_net(const std::shared_ptr<net>& net, bool clear_current_selection)
{
    if(!g_netlist->is_net_in_netlist(net))
        return;

    if(clear_current_selection)
        g_selection_relay.clear();

    g_selection_relay.m_selected_nets.insert(net->get_id());
    g_selection_relay.selection_changed(this);
}

void gui_api::select_net(u32 net_id, bool clear_current_selection)
{
    select_net(g_netlist->get_net_by_id(net_id), clear_current_selection);
}

void gui_api::select_net(const std::vector<std::shared_ptr<net>>& nets, bool clear_current_selection)
{
    QSet<u32> net_ids;

    for(auto net : nets)
    {
        if(!g_netlist->is_net_in_netlist(net))
            return;

        net_ids.insert(net->get_id());    
    }

    if(clear_current_selection)
        g_selection_relay.clear();

    g_selection_relay.m_selected_nets.unite(net_ids);
    g_selection_relay.selection_changed(this);
}

void gui_api::select_net(const std::vector<u32>& net_ids, bool clear_current_selection)
{
    std::vector<std::shared_ptr<net>> nets(net_ids.size());
    std::transform(net_ids.begin(), net_ids.end(), nets.begin(), [](u32 net_id){return g_netlist->get_net_by_id(net_id);});
    select_net(nets, clear_current_selection);
}

void gui_api::select_module(const std::shared_ptr<module>& module, bool clear_current_selection)
{
    if(!g_netlist->is_module_in_netlist(module))
        return;

    if(clear_current_selection)
        g_selection_relay.clear();

    g_selection_relay.m_selected_modules.insert(module->get_id());
    g_selection_relay.selection_changed(this);
}

void gui_api::select_module(u32 module_id, bool clear_current_selection)
{
    select_module(g_netlist->get_module_by_id(module_id), clear_current_selection);
}

void gui_api::select_module(const std::vector<std::shared_ptr<module>>& modules, bool clear_current_selection)
{
    QSet<u32> module_ids;

    for(auto module : modules)
    {
        if(!g_netlist->is_module_in_netlist(module))
            return;

        module_ids.insert(module->get_id());    
    }

    if(clear_current_selection)
        g_selection_relay.clear();

    g_selection_relay.m_selected_modules.unite(module_ids);
    g_selection_relay.selection_changed(this);
}

void gui_api::select_module(const std::vector<u32>& module_ids, bool clear_current_selection)
{
    std::vector<std::shared_ptr<module>> modules(module_ids.size());
    std::transform(module_ids.begin(), module_ids.end(), modules.begin(), [](u32 g_id){return g_netlist->get_module_by_id(g_id);});
    select_module(modules, clear_current_selection);
}

void gui_api::select(const std::shared_ptr<gate>& gate, bool clear_current_selection)
{
    select_gate(gate, clear_current_selection);
}

void gui_api::select(const std::shared_ptr<net>& net, bool clear_current_selection)
{
    select_net(net, clear_current_selection);
}
void gui_api::select(const std::shared_ptr<module>& module, bool clear_current_selection)
{
    select_module(module, clear_current_selection);
}

void gui_api::select(const std::vector<std::shared_ptr<gate>>& gates, bool clear_current_selection)
{
    select_gate(gates, clear_current_selection);
}
void gui_api::select(const std::vector<std::shared_ptr<net>>& nets, bool clear_current_selection)
{
    select_net(nets, clear_current_selection);
}

void gui_api::select(const std::vector<std::shared_ptr<module>>& modules, bool clear_current_selection)
{
    select_module(modules, clear_current_selection);
}

void gui_api::select(const std::vector<u32>& gate_ids, const std::vector<u32>& net_ids, const std::vector<u32>& module_ids, bool clear_current_selection)
{
    if(clear_current_selection)
        g_selection_relay.clear();

    select_gate(gate_ids, false);
    select_net(net_ids, false);
    select_module(module_ids, false);
}

void gui_api::select(const std::vector<std::shared_ptr<gate>>& gates, const std::vector<std::shared_ptr<net>>& nets, const std::vector<std::shared_ptr<module>>& modules, bool clear_current_selection)
{
    if(clear_current_selection)
        g_selection_relay.clear();

    select_gate(gates, false);
    select_net(nets, false);
    select_module(modules, false);
}

void gui_api::deselect_gate(const std::shared_ptr<gate>& gate)
{
    if(!g_netlist->is_gate_in_netlist(gate))
        return;

    g_selection_relay.m_selected_gates.remove(gate->get_id());
    g_selection_relay.selection_changed(this);
}

void gui_api::deselect_gate(u32 gate_id)
{
    deselect_gate(g_netlist->get_gate_by_id(gate_id));
}

void gui_api::deselect_gate(const std::vector<std::shared_ptr<gate>>& gates)
{
    QSet<u32> gate_ids;

    for(std::shared_ptr<gate> gate : gates)
    {
        if(!g_netlist->is_gate_in_netlist(gate))
            return;

        gate_ids.insert(gate->get_id());    
    }

    g_selection_relay.m_selected_gates.subtract(gate_ids);
    g_selection_relay.selection_changed(this);
}

void gui_api::deselect_gate(const std::vector<u32>& gate_ids)
{
    std::vector<std::shared_ptr<gate>> gates(gate_ids.size());
    std::transform(gate_ids.begin(), gate_ids.end(), gates.begin(), [](u32 gate_id){return g_netlist->get_gate_by_id(gate_id);});
    deselect_gate(gates);
}

void gui_api::deselect_net(const std::shared_ptr<net>& net)
{
    if(!g_netlist->is_net_in_netlist(net))
        return;

    g_selection_relay.m_selected_nets.remove(net->get_id());
    g_selection_relay.selection_changed(this);
}

void gui_api::deselect_net(u32 net_id)
{
    deselect_net(g_netlist->get_net_by_id(net_id));
}

void gui_api::deselect_net(const std::vector<std::shared_ptr<net>>& nets)
{
    QSet<u32> net_ids;

    for(std::shared_ptr<net> net : nets)
    {
        if(!g_netlist->is_net_in_netlist(net))
            return;

        net_ids.insert(net->get_id());    
    }

    g_selection_relay.m_selected_nets.subtract(net_ids);
    g_selection_relay.selection_changed(this);
}

void gui_api::deselect_net(const std::vector<u32>& net_ids)
{
    std::vector<std::shared_ptr<net>> nets(net_ids.size());
    std::transform(net_ids.begin(), net_ids.end(), nets.begin(), [](u32 net_id){return g_netlist->get_net_by_id(net_id);});
    deselect_net(nets);
}

void gui_api::deselect_module(const std::shared_ptr<module>& module)
{
    if(!g_netlist->is_module_in_netlist(module))
        return;

    g_selection_relay.m_selected_modules.remove(module->get_id());
    g_selection_relay.selection_changed(this);
}

void gui_api::deselect_module(u32 module_id)
{
    deselect_module(g_netlist->get_module_by_id(module_id));
}

void gui_api::deselect_module(const std::vector<std::shared_ptr<module>>& modules)
{
    QSet<u32> module_ids;

    for(std::shared_ptr<module> module : modules)
    {
        if(!g_netlist->is_module_in_netlist(module))
            return;

        module_ids.insert(module->get_id());    
    }

    g_selection_relay.m_selected_modules.subtract(module_ids);
    g_selection_relay.selection_changed(this);
}

void gui_api::deselect_module(const std::vector<u32>& module_ids)
{
    std::vector<std::shared_ptr<module>> modules(module_ids.size());
    std::transform(module_ids.begin(), module_ids.end(), modules.begin(), [](u32 module_id){return g_netlist->get_module_by_id(module_id);});
    deselect_module(modules);
}

void gui_api::deselect(const std::shared_ptr<gate>& gate)
{
    deselect_gate(gate);
}

void gui_api::deselect(const std::shared_ptr<net>& net)
{
    deselect_net(net);
}
void gui_api::deselect(const std::shared_ptr<module>& module)
{
    deselect_module(module);
}

void gui_api::deselect(const std::vector<std::shared_ptr<gate>>& gates)
{
    deselect_gate(gates);
}
void gui_api::deselect(const std::vector<std::shared_ptr<net>>& nets)
{
    deselect_net(nets);
}

void gui_api::deselect(const std::vector<std::shared_ptr<module>>& modules)
{
    deselect_module(modules);
}

void gui_api::deselect(const std::vector<u32>& gate_ids, const std::vector<u32>& net_ids, const std::vector<u32>& module_ids)
{
    deselect_gate(gate_ids);
    deselect_net(net_ids);
    deselect_module(module_ids);
}

void gui_api::deselect(const std::vector<std::shared_ptr<gate>>& gates, const std::vector<std::shared_ptr<net>>& nets, const std::vector<std::shared_ptr<module>>& modules)
{
    deselect_gate(gates);
    deselect_net(nets);
    deselect_module(modules);
}

void gui_api::deselect_all_items()
{
    g_selection_relay.clear_and_update();
}
