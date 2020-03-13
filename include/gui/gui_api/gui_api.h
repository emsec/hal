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
    static std::vector<u32> get_selected_gate_ids();
    static std::vector<u32> get_selected_net_ids();
    static std::vector<u32> get_selected_module_ids();
    static std::tuple<std::vector<u32>, std::vector<u32>, std::vector<u32>> get_selected_item_ids();

    static std::vector<std::shared_ptr<gate>> get_selected_gates();
    static std::vector<std::shared_ptr<net>> get_selected_nets();
    static std::vector<std::shared_ptr<module>> get_selected_modules();
    static std::tuple<std::vector<std::shared_ptr<gate>>, std::vector<std::shared_ptr<net>>, std::vector<std::shared_ptr<module>>> get_selected_items();

    
    static void print_selected_gates();
    static void print_selected_nets();
    static void print_selected_modules();
    static void print_selected_items();
    

    static void select_gate(u32 gate_id, bool clear_current_selection = true);
    static void select_gate(const std::vector<u32>& gate_ids, bool clear_current_selection = true);
    static void select_gate(const std::shared_ptr<gate>& gate, bool clear_current_selection = true);
    static void select_gate(const std::vector<std::shared_ptr<gate>>& gates, bool clear_current_selection = true);

    static void select_net(u32 net_id, bool clear_current_selection = true);
    static void select_net(const std::vector<u32>& net_ids, bool clear_current_selection = true);
    static void select_net(const std::shared_ptr<net>& net, bool clear_current_selection = true);
    static void select_net(const std::vector<std::shared_ptr<net>>& nets, bool clear_current_selection = true);

    static void select_module(u32 module_id, bool clear_current_selection = true);  
    static void select_module(const std::vector<u32>& module_ids, bool clear_current_selection = true);
    static void select_module(const std::shared_ptr<module>& module, bool clear_current_selection = true);
    static void select_module(const std::vector<std::shared_ptr<module>>& modules, bool clear_current_selection = true);

    static void select(const std::shared_ptr<gate>& gate, bool clear_current_selection = true);
    static void select(const std::shared_ptr<net>& net, bool clear_current_selection = true);
    static void select(const std::shared_ptr<module>& module, bool clear_current_selection = true);
    static void select(const std::vector<std::shared_ptr<gate>>& gates, bool clear_current_selection = true);
    static void select(const std::vector<std::shared_ptr<net>>& nets, bool clear_current_selection = true);
    static void select(const std::vector<std::shared_ptr<module>>& modules, bool clear_current_selection = true);
    static void select(const std::vector<u32>& gate_ids, const std::vector<u32>& net_ids, const std::vector<u32>& module_ids, bool clear_current_selection = true);
    static void select(const std::vector<std::shared_ptr<gate>>& gates, const std::vector<std::shared_ptr<net>>& nets, const std::vector<std::shared_ptr<module>>& modules, bool clear_current_selection = true);

    static void deselect_gate(u32 gate_id);
    static void deselect_gate(const std::vector<u32>& gate_ids);
    static void deselect_gate(const std::shared_ptr<gate>& gate);
    static void deselect_gate(const std::vector<std::shared_ptr<gate>>& gates);

    static void deselect_net(u32 net_id);
    static void deselect_net(const std::vector<u32>& net_ids);
    static void deselect_net(const std::shared_ptr<net>& net);
    static void deselect_net(const std::vector<std::shared_ptr<net>>& nets);

    static void deselect_module(u32 module_id);  
    static void deselect_module(const std::vector<u32>& module_ids);
    static void deselect_module(const std::shared_ptr<module>& module);
    static void deselect_module(const std::vector<std::shared_ptr<module>>& modules);

    static void deselect_all_items();
    static void deselect(const std::shared_ptr<gate>& gate);
    static void deselect(const std::shared_ptr<net>& net);
    static void deselect(const std::shared_ptr<module>& module);
    static void deselect(const std::vector<std::shared_ptr<gate>>& gates);
    static void deselect(const std::vector<std::shared_ptr<net>>& nets);
    static void deselect(const std::vector<std::shared_ptr<module>>& modules);
    static void deselect(const std::vector<u32>& gate_ids, const std::vector<u32>& net_ids, const std::vector<u32>& module_ids );
    static void deselect(const std::vector<std::shared_ptr<gate>>& gates, const std::vector<std::shared_ptr<net>>& nets, const std::vector<std::shared_ptr<module>>& modules);

private:
    gui_api();
};

#endif // GUI_API_H
