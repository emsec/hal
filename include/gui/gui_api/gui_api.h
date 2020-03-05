#ifndef GUI_API_H
#define GUI_API_H

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/module.h"

#include <vector>
#include <tuple>

class gui_api
{
public:
    gui_api();
    ~gui_api();

    std::vector<u32> get_selected_gate_ids();
    std::vector<u32> get_selected_net_ids();
    std::vector<u32> get_selected_module_ids();
    std::tuple<std::vector<u32>, std::vector<u32>, std::vector<u32>> get_selected_item_ids();

    std::vector<std::shared_ptr<gate>> get_selected_gates();
    std::vector<std::shared_ptr<net>> get_selected_nets();
    std::vector<std::shared_ptr<module>> get_selected_modules();
    std::tuple<std::vector<std::shared_ptr<gate>>, std::vector<std::shared_ptr<net>>, std::vector<std::shared_ptr<module>>> get_selected_items();

    void print_selected_gates();
    void print_selected_nets();
    void print_selected_modules();
    void print_selected_items();

    void select_gate(u32 gate_id, bool clear_current_selection = true);
    void select_gate(std::vector<u32> gate_ids, bool clear_current_selection = true);
    void select_gate(std::shared_ptr<gate> gate, bool clear_current_selection = true);
    void select_gate(std::vector<std::shared_ptr<gate>> gates, bool clear_current_selection = true);

    void select_net(u32 net_id, bool clear_current_selection = true);
    void select_net(std::vector<u32> net_ids, bool clear_current_selection = true);
    void select_net(std::shared_ptr<net> net, bool clear_current_selection = true);
    void select_net(std::vector<std::shared_ptr<net>> nets, bool clear_current_selection = true);

    void select_module(u32 module_id, bool clear_current_selection = true);  
    void select_module(std::vector<u32> module_ids, bool clear_current_selection = true);
    void select_module(std::shared_ptr<module> module, bool clear_current_selection = true);
    void select_module(std::vector<std::shared_ptr<module>> modules, bool clear_current_selection = true);

    void select(std::shared_ptr<gate> gate, bool clear_current_selection = true);
    void select(std::shared_ptr<net> net, bool clear_current_selection = true);
    void select(std::shared_ptr<module> module, bool clear_current_selection = true);
    void select(std::vector<std::shared_ptr<gate>> gates, bool clear_current_selection = true);
    void select(std::vector<std::shared_ptr<net>> nets, bool clear_current_selection = true);
    void select(std::vector<std::shared_ptr<module>> modules, bool clear_current_selection = true);
    void select(std::vector<u32> gate_ids, std::vector<u32> net_ids, std::vector<u32> module_ids, bool clear_current_selection = true);
    void select(std::vector<std::shared_ptr<gate>> gates, std::vector<std::shared_ptr<net>> nets, std::vector<std::shared_ptr<module>> modules, bool clear_current_selection = true);

    void clear_gate(u32 gate_id);
    void clear_gate(std::vector<u32> gate_ids);
    void clear_gate(std::shared_ptr<gate> gate);
    void clear_gate(std::vector<std::shared_ptr<gate>> gates);

    void clear_net(u32 net_id);
    void clear_net(std::vector<u32> net_ids);
    void clear_net(std::shared_ptr<net> net);
    void clear_net(std::vector<std::shared_ptr<net>> nets);

    void clear_module(u32 module_id);  
    void clear_module(std::vector<u32> module_ids );
    void clear_module(std::shared_ptr<module> modul);
    void clear_module(std::vector<std::shared_ptr<module>> modules);

    void clear();
    void clear(std::shared_ptr<gate> gate);
    void clear(std::shared_ptr<net> net);
    void clear(std::shared_ptr<module> module);
    void clear(std::vector<std::shared_ptr<gate>> gates);
    void clear(std::vector<std::shared_ptr<net>> nets);
    void clear(std::vector<std::shared_ptr<module>> modules);
    void clear(std::vector<u32> gate_ids, std::vector<u32> net_ids, std::vector<u32> module_ids );
    void clear(std::vector<std::shared_ptr<gate>> gates, std::vector<std::shared_ptr<net>> nets, std::vector<std::shared_ptr<module>> modules);
};

#endif // GUI_API_H
